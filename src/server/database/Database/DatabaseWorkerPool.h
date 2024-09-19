#ifndef DATABASEWORKERPOOL_H_
#define DATABASEWORKERPOOL_H_

#include "DatabaseEnvFwd.h"
#include "Define.h"
#include "StringFormat.h"

#include <array>
#include <vector>

//cient version
#if MARIADB_VERSION_ID >= 100600
#define MIN_MYSQL_CLIENT_VERSION 30203u
#else
#define MIN_MYSQL_CLIENT_VERSION 50700u
#endif

//server version
#define MIN_MYSQL_SERVER_VERSION "5.7.0"

#define MIN_MARIADB_SERVER_VERSION "10.5.0"

template<typename T>
class ProducerConsumerQueue;

class SQLOperation;
struct MySQLConnectionInfo;

template<class T>
class DatabaseWorkerPool
{
private:
    enum InternalIndex
    {
        IDX_ASYNC,
        IDX_SYNCH,
        IDX_SIZE
    };
public:
    DatabaseWorkerPool();
    ~DatabaseWorkerPool();
    void SetConnectionInfo(std::string_view infoString, uint8 const asyncThreads, uint8 const synchThreads);

    uint32 Open();
    void Close();

    bool PrepareStatements();

    [[nodiscard]] inline MySQLConnectionInfo const* GetConnectionInfo() const
    {
        return _connectionInfo.get();
    }

    void Execute(std::string_view sql);

    template<typename... Args>
    void Execute(std::string_view sql, Args&&... args)
    {
        if (sql.empty())
        {
            return ;
        }

        Execute(mpool::StringFormatFmt(sql, std::forward<Args>(args)...));
    }

    void Execute(PreparedStatement<T>* stmt);

    void DirectExecute(std::string_view sql);

    template<typename... Args>
    void DirectExecute(std::string_view sql, Args&&... args)
    {
        if (sql.empty())
        {
            return ;
        }

        DirectExecute(mpool::StringFormatFmt(sql, std::forward<Args>(args)...));
    }

    void DirectExecute(PreparedStatement<T>* stmt);

    QueryResult Query(std::string_view sql);

    template<typename... Args>
    QueryResult Query(std::string_view sql, Args&&... args)
    {
        if (sql.empty())
        {
            return QueryResult(nullptr);
        }

        return Query(mpool::StringFormatFmt(sql, std::forward<Args>(args)...));
    }

//    PreparedQueryResult Query(PreparedStatement<T>* stmt);
    PreparedQueryResult Query(PreparedStatement<T>* stmt);

    QueryCallback AsyncQuery(std::string_view sql);

    QueryCallback AsyncQuery(PreparedStatement<T>* stmt);

    SQLQueryHolderCallback DelayQueryHolder(std::shared_ptr<SQLQueryHolder<T> > holder);

    //transaction
    SQLTransaction<T> BeginTransaction();

    void CommitTransaction(SQLTransaction<T> transaction);

    TransactionCallback AsyncCommitTransaction(SQLTransaction<T> transaction);

    void DirectCommitTransaction(SQLTransaction<T>& transaction);


    void ExecuteOrAppend(SQLTransaction<T>& trans, std::string_view sql);

    void ExecuteOrAppend(SQLTransaction<T>& trans, PreparedStatement<T>* stmt);

    //other
    typedef typename T::Statements PreparedStatementIndex;

    PreparedStatement<T>* GetPreparedStatement(PreparedStatementIndex index);
    PreparedStatement<T>* GetPreparedStatement(std::string_view sql);

    void EscapeString(std::string& str);

    void KeepAlive();

    void WarnAboutSyncQueries([[maybe_unused]] bool warn)
    {
        #ifdef MPOOL_DEBUG
            _warnSyncQueries = warn;
        #endif
    }

    [[nodiscard]] size_t QueueSize() const;
private:
    uint32 OpenConnections(InternalIndex type, uint8 numConnections);

    unsigned long EscapeString(char* to, char const* from, unsigned long length);

    void Enqueue(SQLOperation* op);

    T* GetFreeConnection();

    [[nodiscard]] std::string_view GetDatabaseName() const;

    //queue shared by async worker threads
    std::unique_ptr<ProducerConsumerQueue<SQLOperation*> > _queue;
    std::array<std::vector<std::unique_ptr<T> >, IDX_SIZE> _connections;
    std::unique_ptr<MySQLConnectionInfo> _connectionInfo;
    std::vector<uint8> _preparedStatementSize;
    uint8 _async_threads, _synch_threads;
#ifdef MPOOL_DEBUG
    static inline thread_local bool _warnSyncQueries = false;
#endif
};


#endif