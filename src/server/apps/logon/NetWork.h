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
    CMyTcpConnection(boost::asio::io_context & ser);

    ~CMyTcpConnection();

    typedef std::shared_ptr<CMyTcpConnection> CPMyTcpCon;

    static CPMyTcpCon CreateNew(boost::asio::io_context& io_service);

    void AsyncRead();

    void start();

    void handle_read(boost::system::error_code error, size_t transferredBytes);

    std::string GetRemoteIpAddress() const;

    void printHex(const unsigned char* data, size_t size);

    tcp::socket& socket();
private:

    void handle_write();

    //func
    void HandleWrite(const boost::system::error_code & error, std::size_t bytes_transferred);

    bool VerifyVersion(uint8 const* a, int32 aLength, mpool::Crypto::SHA1::Digest const& versionProof, bool isReconnect = false);

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
    CMyService(const string& strIP, const int& port, int nThreads);

    ~CMyService();

    void Stop();

    void Start();
private:
    boost::asio::io_context& get_io_service();
    //func
    void HandleAccept(const boost::system::error_code& error,
                      std::shared_ptr<CMyTcpConnection> newConnect);

    void StartAccept();

    //property
    boost::asio::io_context m_ioService;
    boost::asio::ip::tcp::acceptor m_tcpAcceptor;
    std::vector<std::shared_ptr<std::thread>> m_listThread;
    std::size_t m_nThreads;
};

#endif //MPOOL_NETWORK_H
