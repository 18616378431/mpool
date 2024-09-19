#include <iostream>
#include <filesystem>
#include <csignal>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include <openssl/evp.h>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <boost/version.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

//time
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Config.h"
#include "StringFormat.h"
#include "Banner.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DatabaseLoader.h"
#include "GitRevision.h"
#include "IoContext.h"
#include "MySQLThreading.h"
#include "Util.h"
#include "Network.h"

#ifndef POOL_CONFIG
#define POOL_CONFIG "logon.conf"
#endif

using boost::asio::ip::tcp;
using namespace boost::program_options;
namespace fs = std::filesystem;

bool StartDB(std::string name);
void StopDB();
void DBTest();
void showBanner(std::string configFile);
std::string GetDateStr();

void DBTest();

int main(int argc, char** argv)
{
    auto configFile = fs::path(sConfigMgr->GetConfigPath() + std::string(POOL_CONFIG));

    sConfigMgr->Configure(configFile.generic_string(), std::vector<std::string>(argv, argv + argc));

    if (!sConfigMgr->LoadAppConfigs())
    {
        return 1;
    }

    //Banner
    //showBanner(configFile.generic_string());
    mpool::Banner::Show("mpool",
        [](std::string_view text)
        {
            std::cout << text << std::endl;
        },
        []()
        {
            std::cout <<
                mpool::StringFormatFmt("Using configuration file:       {}'",
                    sConfigMgr->GetFilename()) << std::endl;
            std::cout <<
                mpool::StringFormatFmt("Using SSL version:              {},(library:{})",
                    OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION)) << std::endl;
            std::cout <<
                mpool::StringFormatFmt("Using boost version:            {}.{}.{}",
                    BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100) << std::endl;
        });


    //pid
    std::string pidFile = sConfigMgr->GetOption<std::string>("PidFile", "");

    std::cout << "pidFile is " << pidFile << std::endl;

    //启动数据库
    if (!StartDB("Login"))
    {
        return 1;
    }

    std::shared_ptr<void> dbHandle(nullptr, [](void *) { StopDB(); });

//    DBTest();

    int port = sConfigMgr->GetOption<int32>("ServerPort", 1234);

    if (port < 0 || port > 0xFFFF)
    {
        std::cout <<
            mpool::StringFormatFmt("port is error, val is {}!", port) << std::endl;
        return 1;
    }

    std::string bindIp = sConfigMgr->GetOption<std::string>("BindIP", "0.0.0.0");

    uint32 nThreads = 1;

    std::cout << bindIp << std::endl;

    CMyService mySer(bindIp, port, nThreads);
    mySer.Start();
    getchar();
    mySer.Stop();


    //格式化日期
    GetDateStr();

//    getchar();

    return 0;
}

void showBanner(std::string configFile)
{
    std::string escape_test = "\"aaaaa'";
    std::cout << "server authserver." << std::endl;
    std::cout << 
        mpool::StringFormatFmt("    Using configuration file:{}, is escape '{}'",
            sConfigMgr->GetFilename(), escape_test) << std::endl;
    std::cout << 
        mpool::StringFormatFmt("    Using SSL version:{},(library:{})",
            OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION)) << std::endl;
    std::cout << 
        mpool::StringFormatFmt("    Using boost version:{}.{}.{}",
            BOOST_VERSION / 100000, BOOST_VERSION / 100 % 1000, BOOST_VERSION % 100) << std::endl;
}

//启动数据库连接
bool StartDB(std::string name)
{
    MySQL::Library_Init();

    std::string const dbString = sConfigMgr->GetOption<std::string>(name + "DatabaseInfo", "");
    if (dbString.empty())
    {
        std::cout <<
            mpool::StringFormatFmt("Database {} not specified in configuration file!", 
                name) << std::endl;
        return false;
    }

    std::cout << "database info is " << dbString << std::endl;

    std::cout << "Begin connect to db" << std::endl;

    DatabaseLoader loader("server.authserver");

    loader.AddDatabase(LoginDatabase, "Login");

    if (!loader.Load())
    {
        return false;
    }

    std::cout << "Started auth database connection pool." << std::endl;

    return true;
}

//关闭数据库连接
void StopDB()
{
    LoginDatabase.Close();
    MySQL::Library_End();
}

//获取当前年-月-日用于记录日志
std::string GetDateStr()
{
    //当前时间戳
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm time_info;

    time_t timer;
    time(&timer);
    // localtime_s(&time_info, &now_time);
    localtime_r(&timer, &time_info);

    std::stringstream ss;
    ss << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S");
    std::string formated_date = ss.str();
    std::cout << formated_date << std::endl;

    return formated_date;
}

void DBTest()
{
    std::cout << "DBTest() sel\n";

    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_TEST);
    stmt->SetData(0, 1);
    PreparedQueryResult result = LoginDatabase.Query(stmt);

    if (!result)
    {
        std::cout << "DBTest() no record\n";
        return ;
    }

    do
    {
        Field* fields           = result->Fetch();
//        uint32 accountId        = fields[0].Get<uint32>();
        std::string accountName = fields[0].Get<std::string>();

        std::cout << "DBTest() accountName:" << accountName << std::endl;
    } while (result->NextRow());

    std::cout << "DBTest() upd\n";

    LoginDatabasePreparedStatement* stmt_upd = LoginDatabase.GetPreparedStatement(LOGIN_UPD_TEST);
    stmt_upd->SetData(0, 3);
    LoginDatabase.Execute(stmt_upd);

    std::string_view test_sql_exe_just = "update test set count = count + 2 where id = ?";
    uint8 paramsCount = 1;
    LoginDatabasePreparedStatement* stmt_upd_just = new LoginDatabasePreparedStatement(nullptr, nullptr, paramsCount);
    stmt_upd_just->SetData(0, 2);

    stmt_upd_just->isPrepared = true;
    stmt_upd_just->sql = test_sql_exe_just;

//    stmt_upd->SetData(1, GetUInt32Value(ITEM_FIELD_FLAGS));
    LoginDatabase.Execute(stmt_upd_just);
//    (stmt_upd_just->GetPreparedConn())->Unlock();

    std::cout << "DBTest() sel just in time prepare sql\n";

    std::string_view test_sql = "select name from test where id in (?, ?, ?)";
    LoginDatabasePreparedStatement* stmt_just = LoginDatabase.GetPreparedStatement(test_sql);
    stmt_just->SetData(0, 1);
    stmt_just->SetData(1, 2);
    stmt_just->SetData(2, 3);

    stmt_just->isPrepared = true;
    PreparedQueryResult test_result = LoginDatabase.Query(stmt_just);

    if (!test_result)
    {
        std::cout << "DBTest() no record\n";
        return ;
    }

    do
    {
        Field* fields           = test_result->Fetch();
//        uint32 accountId        = fields[0].Get<uint32>();
        std::string accountName = fields[0].Get<std::string>();

        std::cout << "DBTest() prepare accountName:" << accountName << std::endl;
    } while (test_result->NextRow());
}