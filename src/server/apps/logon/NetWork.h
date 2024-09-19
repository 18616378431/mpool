#ifndef MPOOL_NETWORK_H
#define MPOOL_NETWORK_H

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include <iostream>
#include <iomanip>

#include "types.h"
#include "DatabaseEnv.h"
#include "SRP6.h"
#include "Util.h"

#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(s) ((boost::asio::io_context&)(s).get_executor().context())
#else
#define GET_IO_SERVICE(s) ((s).get_io_service())
#endif

using std::cout;
using std::endl;
using std::string;
using boost::asio::ip::tcp;
using SHA1 = mpool::Crypto::SHA1;

class CMyTcpConnection : public std::enable_shared_from_this<CMyTcpConnection>
{
public:
    CMyTcpConnection(boost::asio::io_context & ser) :m_nSocket(ser)
    {

    }

    ~CMyTcpConnection()
    {
        cout << "~CMyTcpConnection" << endl;
    }

    typedef std::shared_ptr<CMyTcpConnection> CPMyTcpCon;

    static CPMyTcpCon CreateNew(boost::asio::io_context& io_service)
    {
        return CPMyTcpCon(new CMyTcpConnection(io_service));
    }

    void AsyncRead()
    {
        m_nSocket.async_read_some(boost::asio::buffer(buff, buff_len),
                                  std::bind(&CMyTcpConnection::handle_read,
                                            this->shared_from_this(),
                                            std::placeholders::_1,
                                            std::placeholders::_2));
    }

    void start()
    {
        cout << "start()" << endl;

        AsyncRead();
    }

    void handle_read(boost::system::error_code error, size_t transferredBytes)
    {
        cout << "handle_read transferredBytes:" << transferredBytes << endl;

        if(!error)
        {
            std::cout << "std::string(buff, transferredBytes):" << std::string(buff, transferredBytes) << std::endl;

            switch (buff[0])
            {
                case AUTH_LOGON_CHALLENGE://first packet
                {
                    sAuthLogonChallenge_C* challenge = reinterpret_cast<sAuthLogonChallenge_C*>(buff);

                    if (challenge->size - (sizeof(sAuthLogonChallenge_C) - AUTH_LOGON_CHALLENGE_INITIAL_SIZE - 1) != challenge->I_len)
                    {
                        cout << "数据包大小有误" << endl;
                        return ;
                    }

                    std::string login((char const*)challenge->I, challenge->I_len);

                    cout << "username:" << login << endl;

                    std::array<char, 5> os;
                    os.fill('\0');
                    memcpy(os.data(), challenge->os, sizeof(challenge->os));
                    _os = os.data();
                    std::reverse(_os.begin(), _os.end());

                    _localizationName.resize(4);
                    for (int i = 0; i < 4; ++i)
                        _localizationName[i] = challenge->country[4 - i - 1];

                    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_LOGONCHALLENGE);
                    stmt->SetData(0, login);

                    PreparedQueryResult result = LoginDatabase.Query(stmt);

                    sAuthLogonChallenge_S first_feedback;

                    first_feedback.cmd = uint8(AUTH_LOGON_CHALLENGE);
                    first_feedback.unk2 = uint8(0x00);

                    if (!result)
                    {
                        first_feedback.error = uint8(WOW_FAIL_UNKNOWN_ACCOUNT);

                        data_len = 3;

                        memcpy(buff, &first_feedback, data_len);
                        handle_write();

                        std::cout << "account no record\n";
                        return ;
                    }

//                    do
//                    {
                        Field* fields           = result->Fetch();

                        uint32 id               = fields[0].Get<uint32>();
                        UserName    = fields[1].Get<std::string>();
                        std::string last_ip     = fields[2].Get<std::string>();
                        std::string salt        = fields[3].Get<std::string>();
                        std::string verifier    = fields[4].Get<std::string>();

                        std::cout << "id:" << id << std::endl;
                        std::cout << "accountName:" << UserName << std::endl;
                        std::cout << "last_ip:" << last_ip << std::endl;
                        std::cout << "salt:" << salt << std::endl;
                        std::cout << "verifier:" << verifier << std::endl;
//                    } while (result->NextRow());

                    _srp6.emplace(UserName,
                                  fields[3].Get<Binary, mpool::Crypto::SRP6::SALT_LENGTH>(),
                                  fields[4].Get<Binary, mpool::Crypto::SRP6::VERIFIER_LENGTH>());

                    //success
                    first_feedback.error = uint8(WOW_SUCCESS);

                    std::memcpy((char *)first_feedback.B, (char *)_srp6->B.data(), 32);

                    first_feedback.g_len = uint8(1);
                    std::strncpy((char *)first_feedback.g, (char *)_srp6->g.data(), 1);

                    first_feedback.N_len = uint8(32);
                    std::strncpy((char *)first_feedback.N, (char *)_srp6->N.data(), 32);

                    std::strncpy((char *)first_feedback.s, (char *)_srp6->s.data(), 32);

                    std::array<uint8, 16> VersionChallenge = { { 0xBA, 0xA3, 0x1E, 0x99, 0xA0, 0x0B, 0x21, 0x57, 0xFC, 0x37, 0x3F, 0xB3, 0x69, 0xCD, 0xD2, 0xF1 } };
                    std::strncpy((char *)first_feedback.unk3, (char *)VersionChallenge.data(), VersionChallenge.size());

                    uint8 securityFlags = 0;
                    first_feedback.N_len = uint8(securityFlags);

                    data_len = sizeof(first_feedback);
                    memcpy(buff, &first_feedback, data_len);
                    handle_write();
                }
                    break;
                case AUTH_LOGON_PROOF:
                {
                    cout << "AUTH_LOGON_PROOF" << endl;
                    sAuthLogonProof_C* logonProof = reinterpret_cast<sAuthLogonProof_C*>(buff);
                    cout << "logonProof.cmd:" << logonProof->cmd << endl;

                    sAuthLogonProof_S proofS;

                    mpool::Crypto::SRP6::EphemeralKey A;
                    memcpy(A.data(), logonProof->A, 32);

                    SHA1::Digest M1;
                    memcpy(M1.data(), logonProof->M1, 20);

                    if (Optional<SessionKey> K = _srp6->VerifyChallengeResponse(A, M1))
                    {
                        std::cout << "K success" << std::endl;
                        _sessionKey = *K;

                        std::array<uint8, 20> crc_hash;
                        memcpy(crc_hash.data(), logonProof->crc_hash, 20);

                        if (!VerifyVersion(logonProof->A, 32, crc_hash, false))
                        {
                            proofS.cmd = uint8(AUTH_LOGON_PROOF);
                            proofS.error = uint8(WOW_FAIL_VERSION_INVALID);

                            data_len = 2;

                            memcpy(buff, &proofS, data_len);
                            handle_write();

                            std::cout << "VerifyVersion check fail\n";

                            return ;
                        }

                        std::cout << "update login info\n";

                        std::string address = GetRemoteIpAddress();
                        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_LOGONPROOF);
                        stmt->SetData(0, _sessionKey);
                        stmt->SetData(1, address);
                        stmt->SetData(2, _localizationName);
                        stmt->SetData(3, _os);
                        stmt->SetData(4, UserName);
                        LoginDatabase.DirectExecute(stmt);

                        //generic M2
                        mpool::Crypto::SHA1::Digest M2 = mpool::Crypto::SRP6::GetSessionVerifier(A, M1, _sessionKey);

                        sAuthLogonProof_S proofS;

                        proofS.cmd = uint8(AUTH_LOGON_PROOF);
                        proofS.error = uint8(0);
                        std::memcpy(proofS.M2, M2.data(), 20);
                        proofS.accountFlags = 0x00800000;
                        proofS.surveyId = 0;
                        proofS.unkFlags = 0;

                        data_len = sizeof(proofS);

                        memcpy(buff, &proofS, data_len);
                        handle_write();
                    }
                    else
                    {
                        proofS.cmd = uint8(AUTH_LOGON_PROOF);
                        proofS.error = uint8(WOW_FAIL_UNKNOWN_ACCOUNT);

                        data_len = 2;

                        memcpy(buff, &proofS, data_len);
                        handle_write();

                        return ;
                    }
                }
                    break;
                default:
                    break;
            }

            AsyncRead();
        }
        else
        {
            m_nSocket.close();
        }
    }

    std::string GetRemoteIpAddress() const
    {
        return m_nSocket.remote_endpoint().address().to_string();
    }

    void printHex(const unsigned char* data, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            // 使用std::hex和std::setw来格式化输出
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
        }

        std::cout << std::endl;
    }

    tcp::socket& socket()
    {
        return m_nSocket;
    }
private:

    void handle_write()
    {
        cout << "buff:" << std::string(buff, data_len) << endl;
        boost::asio::async_write(m_nSocket, boost::asio::buffer(buff, buff_len),
                                 boost::bind(&CMyTcpConnection::HandleWrite, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
        AsyncRead();
    }

    //func
    void HandleWrite(const boost::system::error_code & error, std::size_t bytes_transferred)
    {
        if (!error)
        {
            boost::mutex::scoped_lock lk(m_ioMutex);
            cout << "bytes_transferred=" << bytes_transferred << ",thread id=" << boost::this_thread::get_id() << endl;
        }
        else
        {
            cout << "connection close" << endl;
        }
    }

    bool VerifyVersion(uint8 const* a, int32 aLength, mpool::Crypto::SHA1::Digest const& versionProof, bool isReconnect = false)
    {
        cout << "VerifyVersion" << endl;

        mpool::Crypto::SHA1::Digest zeros{};
        mpool::Crypto::SHA1::Digest const* versionHash{ nullptr };

        if (!isReconnect)
        {
            if (_os == "Win")
            {
                HexStrToByteArray(winCheckSumSeed, WindowsHash);
                versionHash = &WindowsHash;
            }
            else if (_os == "OSX")
            {
                HexStrToByteArray(macCheckSumSeed, MacHash);
                versionHash = &MacHash;
            }

            if (zeros == *versionHash)
                return true;                                                            // not filled serverside
        }
        else
            versionHash = &zeros;

        mpool::Crypto::SHA1 version;
        version.UpdateData(a, aLength);
        version.UpdateData(*versionHash);
        version.Finalize();

        cout << "VerifyVersion result:" << (versionProof == version.GetDigest()) << endl;

        return (versionProof == version.GetDigest());
    }

    //porperty
    tcp::socket m_nSocket;
    boost::mutex m_ioMutex;
    char buff[1024];
    int buff_len = 1024;
    int data_len = 0;
    Optional<mpool::Crypto::SRP6> _srp6;
    SessionKey _sessionKey = {};
    std::string winCheckSumSeed = "CDCBBD5188315E6B4D19449D492DBCFAF156A347";
    std::string macCheckSumSeed = "B706D13FF2F4018839729461E3F8A0E2B5FDC034";
    std::string _os;
    std::array<uint8, 20> WindowsHash;
    std::array<uint8, 20> MacHash;
    std::string _localizationName;
    std::string UserName;
};

class CMyService : private boost::noncopyable
{
public:
    CMyService(const string& strIP, const int& port, int nThreads)
            :m_tcpAcceptor(m_ioService),
             m_nThreads(nThreads)
    {
        tcp::resolver resolver(m_ioService);
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

        m_tcpAcceptor.open(endpoint.protocol());
        m_tcpAcceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_tcpAcceptor.bind(endpoint);
        m_tcpAcceptor.listen();

        StartAccept();
    }

    ~CMyService()
    {
        Stop();
    }

    void Stop()
    {
        m_ioService.stop();
        for (auto it = m_listThread.cbegin();
                it != m_listThread.cend(); ++it)
        {
            (*it)->join();
        }
    }

    void Start()
    {
        for (int i = 0; i != m_nThreads; i++)
        {
            std::shared_ptr<std::thread> pTh(new std::thread(
                    std::bind(&boost::asio::io_context::run, &m_ioService)));
            m_listThread.push_back(pTh);
        }
    }
private:
    boost::asio::io_context& get_io_service()
    {
        return GET_IO_SERVICE(m_tcpAcceptor);
    }
    //func
    void HandleAccept(const boost::system::error_code& error,
                      std::shared_ptr<CMyTcpConnection> newConnect)
    {
        StartAccept();

        if (!error)
        {
            std::cout << "HandleAccept thread id:" << boost::this_thread::get_id() << std::endl;
            //boost::this_thread::sleep(boost::chrono::seconds(20));
            newConnect->start();
        }
    }

    void StartAccept()
    {
        CMyTcpConnection::CPMyTcpCon newConnect = CMyTcpConnection::CreateNew(get_io_service());
        m_tcpAcceptor.async_accept(newConnect->socket(),
                                   boost::bind(&CMyService::HandleAccept, this,
                                               boost::asio::placeholders::error, newConnect));
    }

    //property
    boost::asio::io_context m_ioService;
    boost::asio::ip::tcp::acceptor m_tcpAcceptor;
    std::vector<std::shared_ptr<std::thread>> m_listThread;
    std::size_t m_nThreads;
};

#endif //MPOOL_NETWORK_H
