#include "DatabaseWorkerPool.h"
#include "AdhocStatement.h"
#include "DatabaseEnvFwd.h"
#include "Errors.h"
// #include "Log.h"
#include "LoginDatabase.h"
#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"
#include "MySQLWorkaround.h"
#include "PCQueue.h"
#include "PreparedStatement.h"
#include "QueryCallback.h"
#include "QueryHolder.h"
#include "QueryResult.h"
#include "SQLOperation.h"
#include "Transaction.h"
// #include "WorldDatabase.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <mysqld_error.h>
#include <sstream>
#include <string_view>
#include <vector>
#include <iostream>

#ifdef MPOOL_DEBUG
#include <boost/stacktrace.hpp>
#include <sstream>
#endif

class PingOperation : public SQLOperation
{
    bool Execute() override
    {
        m_conn->Ping();
        return true;
    }
};

template< class T>
DatabaseWorkerPool<T>::DatabaseWorkerPool() :
    _queue(new ProducerConsumerQueue<SQLOperation*>()),
    _async_threads(0),
    _synch_threads(0)
{
    WPFatal(mysql_thread_safe(), "Used MySQL library isn't thread safe.");

#if !defined (MARIADB_VERSION_ID) || MARIADB_VERSION_ID < 100600
    bool isSupportClientDB = mysql_get_client_version() >= MIN_MYSQL_CLIENT_VERSION;
    bool isSameClientDB = mysql_get_client_version() == MYSQL_VERSION_ID;
#else
    bool isSupportClientDB = mysql_get_client_version() >= MIN_MYSQL_CLIENT_VERSION;
    bool isSameClientDB = true;
#endif

    WPFatal(isSupportClientDB, "mpool does not support MySQL version below 5.7 or MariaDB versions below 10.5\n\nFound version: {} / {}.Server compiled with: {}.",
        mysql_get_client_info(), mysql_get_client_version(), MYSQL_VERSION_ID);
    WPFatal(isSameClientDB, "Used MySQL library version: ({} id {}) does not match the version id used to compile mpool (id {}).",
        mysql_get_client_info(), mysql_get_client_version(), MYSQL_VERSION_ID);
}

template<class T>
DatabaseWorkerPool<T>::~DatabaseWorkerPool()
{
    _queue->Cancel();
}

template<class T>
void DatabaseWorkerPool<T>::SetConnectionInfo(std::string_view infoString, uint8 const asyncThreads, uint8 const synchThreads)
{
    _connectionInfo = std::make_unique<MySQLConnectionInfo>(infoString);

    _async_threads = asyncThreads;
    _synch_threads = synchThreads;
}

template<class T>
uint32 DatabaseWorkerPool<T>::Open()
{
    WPFatal(_connectionInfo.get(), "ConnectionInfo is not set");

    std::cout << mpool::StringFormatFmt("Opening DatabasePool '{}'. Asynchronous connections: {}, synchronous connections: {}.",
                GetDatabaseName(), _async_threads, _synch_threads) << std::endl;

    uint32 error = OpenConnections(IDX_ASYNC, _async_threads);

    if (error)
    {
        return error;
    }

    error = OpenConnections(IDX_SYNCH, _synch_threads);

    if (!error)
    {
        std::cout << mpool::StringFormatFmt("DatabasePool '{}' opened successfully.{} total connections running.",
                GetDatabaseName(), (_connections[IDX_SYNCH].size() + _connections[IDX_ASYNC].size())) << std::endl;
    }

    std::cout << " " << std::endl;

    return error;
}

template<class T>
void DatabaseWorkerPool<T>::Close()
{
    std::cout << mpool::StringFormatFmt("Closing down DatabasePool '{}'.", GetDatabaseName()) << std::endl;

    _connections[IDX_ASYNC].clear();

    std::cout << mpool::StringFormatFmt("Asynchronous connections on DatabasePool '{}' terminated.Proceeding iwth synchronous connections", GetDatabaseName()) << std::endl;

    _connections[IDX_SYNCH].clear();

    std::cout << mpool::StringFormatFmt("All connections on DatabasePool '{}' closed.", GetDatabaseName()) << std::endl;
}

template<class T>
bool DatabaseWorkerPool<T>::PrepareStatements()
{
    for (auto const& connections : _connections)
    {
        for (auto const& connection : connections)
        {
            connection->LockIfReady();
            if (!connection->PrepareStatements())
            {
                connection->Unlock();
                Close();
                return false;
            }
            else
            {
                connection->Unlock();
            }
            
            size_t const preparedSize = connection->m_stmts.size();
            
            if (_preparedStatementSize.size() < preparedSize)
            {
                _preparedStatementSize.resize(preparedSize);
            }

            for (size_t i = 0; i < preparedSize; ++i)
            {
                if (_preparedStatementSize[i] > 0)
                {
                    continue;
                }

                if (MySQLPreparedStatement* stmt = connection->m_stmts[i].get())
                {
                    uint32 const paramCount = stmt->GetParameterCount();

                    ASSERT(paramCount < std::numeric_limits<uint8>::max());

                    _preparedStatementSize[i] = static_cast<uint8>(paramCount);
                }
            }
        }
    }

    return true;
}

template<class T>
QueryResult DatabaseWorkerPool<T>::Query(std::string_view sql)
{
    auto connection = GetFreeConnection();

    ResultSet* result = connection->Query(sql);
    connection->Unlock();

    if (!result || !result->GetRowCount() || !result->NextRow())
    {
        delete result;
        return QueryResult(nullptr);
    }

    return QueryResult(result);
}

template<class T>
PreparedQueryResult DatabaseWorkerPool<T>::Query(PreparedStatement<T>* stmt)
{
    T* connection = nullptr;

    if (stmt->isPrepared)
    {
        connection = stmt->GetPreparedConn();
    }
    else
    {
        connection = GetFreeConnection();
    }

    PreparedResultSet* ret = connection->Query(stmt);
    connection->Unlock();

    delete stmt;

    if (!ret || !ret->GetRowCount())
    {
        delete ret;
        return PreparedQueryResult(nullptr);
    }

    return PreparedQueryResult(ret);
}

template<class T>
QueryCallback DatabaseWorkerPool<T>::AsyncQuery(std::string_view sql)
{
    BasicStatementTask* task = new BasicStatementTask(sql, true);
    QueryResultFuture result = task->GetFuture();
    Enqueue(task);
    return QueryCallback(std::move(result));
}

template<class T>
QueryCallback DatabaseWorkerPool<T>::AsyncQuery(PreparedStatement<T>* stmt)
{
    PreparedStatementTask* task = new PreparedStatementTask(stmt, true);
    PreparedQueryResultFuture result = task->GetFuture();
    Enqueue(task);
    return QueryCallback(std::move(result));
}

template<class T>
SQLQueryHolderCallback DatabaseWorkerPool<T>::DelayQueryHolder(std::shared_ptr<SQLQueryHolder<T>> holder)
{
    SQLQueryHolderTask* task = new SQLQueryHolderTask(holder);
    QueryResultHolderFuture result = task->GetFuture();
    Enqueue(task);
    return { std::move(holder), std::move(result) };
}

template<class T>
SQLTransaction<T> DatabaseWorkerPool<T>::BeginTransaction()
{
    return std::make_shared<Transaction<T>>();
}

template<class T>
void DatabaseWorkerPool<T>::CommitTransaction(SQLTransaction<T> transaction)
{
#ifdef MPOOL_DEBUG
    switch (transaction->GetSize())
    {
        case 0:
            std::cout << 
            mpool::StringFormatFmt("Transaction contains 0 queries.Not Executing") << std::endl;
        break;
        case 1:
        std::cout << mpool::StringFormatFmt("Warning: Transaction only holds 1 query, consider removing Transaction context in code") << std::endl;
        break;
        default:
        break;
    }
#endif
    Enqueue(new TransactionTask(transaction));
}

template<class T>
TransactionCallback DatabaseWorkerPool<T>::AsyncCommitTransaction(SQLTransaction<T> transaction)
{
    #ifdef MPOOL_DEBUG
    switch (transaction->GetSize())
    {
        case 0:
            std::cout << 
            mpool::StringFormatFmt("Transaction contains 0 queries.Not Executing") << std::endl;
        break;
        case 1:
        std::cout << mpool::StringFormatFmt("Warning: Transaction only holds 1 query, consider removing Transaction context in code") << std::endl;
        break;
        default:
        break;
    }
#endif

    TransactionWithResultTask* task = new TransactionWithResultTask(transaction);
    TransactionFuture result = task->GetFuture();
    Enqueue(task);
    return TransactionCallback(std::move(result));
}

template<class T>
void DatabaseWorkerPool<T>::DirectCommitTransaction(SQLTransaction<T>& transaction)
{
    T* connection = GetFreeConnection();
    int errorCode = connection->ExecuteTransaction(transaction);

    if (!errorCode)
    {
        connection->Unlock();
        return ;
    }

    if (errorCode == ER_LOCK_DEADLOCK)
    {
        uint8 loopBreaker = 5;

        for (uint8 i = 0; i < loopBreaker; ++i)
        {
            if (!connection->ExecuteTransaction(transaction))
            {
                break;
            }
        }
    }

    transaction->Cleanup();

    connection->Unlock();
}

template<class T>
PreparedStatement<T>* DatabaseWorkerPool<T>::GetPreparedStatement(PreparedStatementIndex index)
{
    return new PreparedStatement<T>(index, _preparedStatementSize[index]);
}

template<class T>
PreparedStatement<T>* DatabaseWorkerPool<T>::GetPreparedStatement(std::string_view sql)
{
    T* connection = GetFreeConnection();

    std::shared_ptr<MySQLPreparedStatement> mysql_stmt= connection->PrepareStatement(sql, CONNECTION_SYNCH);

    uint32 const paramCount = mysql_stmt->GetParameterCount();

    ASSERT(paramCount < std::numeric_limits<uint8>::max());

    uint8 _preparedStatementParamsCount = static_cast<uint8>(paramCount);

    std::cout << "sql:" << sql << ",paramcount:" << paramCount << std::endl;

    return new PreparedStatement<T>(connection, mysql_stmt, _preparedStatementParamsCount);
}

template<class T>
void DatabaseWorkerPool<T>::EscapeString(std::string& str)
{
    if (str.empty())
    {
        return ;
    }

    char* buf = new char[str.size() * 2 + 1];
    EscapeString(buf, str.c_str(), uint32(str.size()));
    str = buf;
    delete[] buf;
}

template<class T>
void DatabaseWorkerPool<T>::KeepAlive()
{
    for (auto& connection : _connections[IDX_SYNCH])
    {
        if (connection->LockIfReady())
        {
            connection->Ping();
            connection->Unlock();
        }
    }

    auto const count = _connections[IDX_ASYNC].size();

    for (uint8 i = 0; i < count; ++i)
    {
        Enqueue(new PingOperation);
    }
}

bool DatabaseIncompatibleVersion(std::string const mysqlVersion)
{
    auto parse = [](std::string const& input)
    {
        std::vector<uint8> result;
        std::istringstream parser(input);
        result.push_back(parser.get());
        
        for (int i = 1; i < 3; i++)
        {
            parser.get();
            result.push_back(parser.get());
        }

        return result;
    };

    uint8 offset = 0;
    std::string minVersion = MIN_MYSQL_SERVER_VERSION;

    if (mysqlVersion.find("MariaDB") != std::string::npos)
    {
        if (mysqlVersion.rfind("5.5.5-", 0) == 0)
        {
            offset = 6;
        }

        minVersion = MIN_MARIADB_SERVER_VERSION;
    }

    auto parsedMySQLVersion = parse(mysqlVersion.substr(offset));
    auto parsedMinVersion = parse(minVersion);

    return std::lexicographical_compare(parsedMySQLVersion.begin(), parsedMySQLVersion.end(),
                    parsedMinVersion.begin(), parsedMinVersion.end());
}

template<class T>
uint32 DatabaseWorkerPool<T>::OpenConnections(InternalIndex type, uint8 numConnections)
{
    for (uint8 i = 0; i < numConnections; ++i)
    {
        auto connection = [&]
        {
            switch (type)
            {
                case IDX_ASYNC:
                    return std::make_unique<T>(_queue.get(), *_connectionInfo);
                break;
                case IDX_SYNCH:
                    return std::make_unique<T>(*_connectionInfo);
                break;
                default:
                ABORT();
            }
        }();

        if (uint32 error = connection->Open())
        {
            _connections[type].clear();
            return error;
        }
        else if (DatabaseIncompatibleVersion(connection->GetServerInfo()))
        {
            std::cout << 
            mpool::StringFormatFmt("mpool does not support MySQL version below 5.7 or MariaDB versions below 10.5\n\nFound version: {} / {}.Server compiled with: {}.",
                    connection->GetServerInfo(), MYSQL_VERSION_ID) << std::endl;
            
            return 1;
        }
        else
        {
            _connections[type].push_back(std::move(connection));
        }
    }

    return 0;
}

template<class T>
unsigned long DatabaseWorkerPool<T>::EscapeString(char* to, char const* from, unsigned long length)
{
    if (!to || !from || !length)
    {
        return 0;
    }

    return _connections[IDX_SYNCH].front()->EscapeString(to, from, length);
}

template<class T>
void DatabaseWorkerPool<T>::Enqueue(SQLOperation* op)
{
    _queue->Push(op);
}

template<class T>
size_t DatabaseWorkerPool<T>::QueueSize() const
{
    return _queue->Size();
}

template<class T>
T* DatabaseWorkerPool<T>::GetFreeConnection()
{
#ifdef MPOOL_DEBUG
    if (_warnSyncQueries)
    {
        std::ostringstream ss;
        ss << boost::stacktrace::stacktrace();
        std::cout << 
            mpool::StringFormatFmt("Sync query at: \n{}",
                    ss.str()) << std::endl;
    }
#endif

    uint8 i = 0;
    auto const num_cons = _connections[IDX_SYNCH].size();
    T* connection = nullptr;

    for (;;)
    {
        connection = _connections[IDX_SYNCH][++i % num_cons].get();
        if (connection->LockIfReady())
        {
            break;
        }
    }

    return connection;
}

template<class T>
std::string_view DatabaseWorkerPool<T>::GetDatabaseName() const
{
    return std::string_view{ _connectionInfo->database };
}

template<class T>
void DatabaseWorkerPool<T>::Execute(std::string_view sql)
{
    if (sql.empty())
    {
        return ;
    }

    BasicStatementTask* task = new BasicStatementTask(sql);
    Enqueue(task);
}

template<class T>
void DatabaseWorkerPool<T>::Execute(PreparedStatement<T>* stmt)
{
//    PreparedStatementTask* task = new PreparedStatementTask(stmt, true);//has result
    PreparedStatementTask* task = new PreparedStatementTask(stmt);//execute
    Enqueue(task);
}

template<class T>
void DatabaseWorkerPool<T>::DirectExecute(std::string_view sql)
{
    if (sql.empty())
    {
        return ;
    }

    T* connection = GetFreeConnection();
    connection->Execute(sql);
    connection->Unlock();
}

template<class T>
void DatabaseWorkerPool<T>::DirectExecute(PreparedStatement<T>* stmt)
{
    T* connection = GetFreeConnection();
    connection->Execute(stmt);
    connection->Unlock();

    delete stmt;
}

template<class T>
void DatabaseWorkerPool<T>::ExecuteOrAppend(SQLTransaction<T>& trans, std::string_view sql)
{
    if (!trans)
    {
        Execute(sql);
    }
    else
    {
        trans->Append(sql);
    }
}

template<class T>
void DatabaseWorkerPool<T>::ExecuteOrAppend(SQLTransaction<T>& trans, PreparedStatement<T>* stmt)
{
    if (!trans)
    {
        Execute(stmt);
    }
    else
    {
        trans->Append(stmt);
    }
}

template class POOL_DATABASE_API DatabaseWorkerPool<LoginDatabaseConnection>;