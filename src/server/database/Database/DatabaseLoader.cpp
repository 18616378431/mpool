#include "DatabaseLoader.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Duration.h"
//#include "Log.h"
#include "StringFormat.h"

#include <errmsg.h>
#include <mysqld_error.h>
#include <thread>
#include <iostream>

DatabaseLoader::DatabaseLoader(const std::string & logger, uint32 const defaultUpdateMask,
                               std::string_view modulesList)
    : _logger(logger),
    _modulesList(modulesList)
{

}

template<class T>
DatabaseLoader& DatabaseLoader::AddDatabase(DatabaseWorkerPool<T> &pool, const std::string &name)
{
    _open.push([this, name, &pool]() -> bool
   {
        const std::string dbString = sConfigMgr->GetOption<std::string>(name + "DatabaseInfo", "");

        if (dbString.empty())
        {
            std::cout << mpool::StringFormatFmt("Database {} not specified in config file", name) << std::endl;

            return false;
        }

        uint8 const asyncThreads = sConfigMgr->GetOption<uint8>(name + "Database.WorkerThreads", 1);

        if (asyncThreads < 1 || asyncThreads > 32)
        {
            std::cout << mpool::StringFormatFmt("{Database: invalid number of woker threads specified,1-32}", name) << std::endl;

            return false;
        }

        const uint8 synchThreads = sConfigMgr->GetOption<uint8>(name + "Database.SynchThreads", 1);

        pool.SetConnectionInfo(dbString, asyncThreads, synchThreads);

        if (uint32 error = pool.Open())
        {
            //try reconnect
            if (error == CR_CONNECTION_ERROR)
            {
                uint8 const attempts = sConfigMgr->GetOption<uint8>("Database.Reconnect.Attempts", 20);
                Seconds reconnectSeconds = Seconds(sConfigMgr->GetOption<uint8>("Database.Reconnect.Seconds", 15));
                uint8 reconnectCount = 0;

                while (reconnectCount < attempts)
                {
                    std::cout << mpool::StringFormatFmt("Retrying after {} seconds", static_cast<uint32>(reconnectSeconds.count())) << std::endl;
                    std::this_thread::sleep_for(reconnectSeconds);
                    error = pool.Open();

                    if (error == CR_CONNECTION_ERROR)
                    {
                        reconnectCount++;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            //Database not exists
            if (error == ER_BAD_DB_ERROR)
            {
                std::cout << "db not exists" << std::endl;
                return false;
            }

            if (error)
            {
                std::cout << mpool::StringFormatFmt("DatabaseWorkerPool {} not opend!", name) << std::endl;

                return false;
            }
        }

       _close.push([&pool]
       {
           pool.Close();
       });

       return true;
   });

    _prepare.push([this, name, &pool]() -> bool
    {
        if (!pool.PrepareStatements())
        {
            std::cout << mpool::StringFormatFmt("Could not prepare statements of the database", name) << std::endl;

            return false;
        }

        return true;
    });

    return *this;
}

bool DatabaseLoader::Load()
{
    if (!OpenDatabases())
    {
        std::cout << mpool::StringFormatFmt("OpenDatabase Fail") << std::endl;
        return false;
    }

    if (!PrepareStatements())
    {
        std::cout << mpool::StringFormatFmt("PrepareStatements Fail") << std::endl;
        return false;
    }

    return true;
}

bool DatabaseLoader::OpenDatabases()
{
    return Process(_open);
}

bool DatabaseLoader::PrepareStatements()
{
    return Process(_prepare);
}

bool DatabaseLoader::Process(std::queue<Predicate> &queue)
{
    while (!queue.empty())
    {
        if (!queue.front()())
        {
            while (!_close.empty())
            {
                _close.top()();
                _close.pop();
            }

            return false;
        }
        queue.pop();
    }

    return true;
}

template POOL_DATABASE_API
DatabaseLoader& DatabaseLoader::AddDatabase<LoginDatabaseConnection>(DatabaseWorkerPool<LoginDatabaseConnection>&, const std::string&);
