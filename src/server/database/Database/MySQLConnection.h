#ifndef _MYSQLCONNECTION_H
#define _MYSQLCONNECTION_H

#include "DatabaseEnvFwd.h"
#include "Define.h"
#include "PCQueue.h"
#include "PreparedStatement.h"
#include "SQLOperation.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>


template <typename T>
class ProducerConsumerQueue;

class DatabaseWorker;
class MySQLPreparedStatement;
class SQLOperation;

enum ConnectionFlags
{
	CONNECTION_ASYNC = 0x1,
	CONNECTION_SYNCH = 0x2,
	CONNECTION_BOTH = CONNECTION_ASYNC | CONNECTION_SYNCH,
};

struct POOL_DATABASE_API MySQLConnectionInfo
{
	explicit MySQLConnectionInfo(std::string_view infoString);

	std::string user;
	std::string password;
	std::string database;
	std::string host;
	std::string port_or_socket;
	std::string ssl;
};


class POOL_DATABASE_API MySQLConnection
{
	template<typename T>
	friend class DatabaseWorkerPool;

	friend class PingOperation;

	friend class PreparedStatementTask;

public:
	MySQLConnection(MySQLConnectionInfo& connInfo);
	MySQLConnection(ProducerConsumerQueue<SQLOperation*>* queue, MySQLConnectionInfo& connInfo);
	virtual ~MySQLConnection();

	virtual uint32 Open();
	void Close();

	bool PrepareStatements();

	bool Execute(std::string_view  sql);
	bool Execute(PreparedStatementBase* stmt);
	ResultSet* Query(std::string_view sql);
//	PreparedResultSet* Query(PreparedStatementBase* stmt);
    PreparedResultSet* Query(PreparedStatementBase* stmt);
	bool _Query(std::string_view sql, MySQLResult** pResult, MySQLField** pFields, uint64* pRowCount, uint32* pFieldCount);
	bool _Query(PreparedStatementBase* stmt, MySQLPreparedStatement** mysqlStmt, MySQLResult** pResult, uint64* pRowCount, uint32* pFieldCountbool);

	void BeginTransaction();
	void RollbackTransaction();
	void CommitTransaction();
	int ExecuteTransaction(std::shared_ptr<TransactionBase> transaction);
	size_t EscapeString(char* to, const char* from, size_t length);
	void Ping();

	uint32 GetLastError();

    void Unlock();
protected:
	bool LockIfReady();

	[[nodiscard]] uint32 GetServerVersion() const;
	[[nodiscard]] std::string GetServerInfo() const;
	MySQLPreparedStatement* GetPreparedStatement(uint32 index);
	void PrepareStatement(uint32 index, std::string_view sql, ConnectionFlags flags);
    std::shared_ptr<MySQLPreparedStatement> PrepareStatement(std::string_view sql, ConnectionFlags flags);

	virtual void DoPrepareStatements() = 0;
	virtual bool _HandleMySQLErrno(uint32 errNo, uint8 attempts = 5);

	typedef std::vector<std::unique_ptr<MySQLPreparedStatement>> PreparedStatementContainer;

	PreparedStatementContainer m_stmts;//MySQLPrepareStatement storage

	bool m_reconnecting;
	bool m_prepareError;
	MySQLHandle* m_Mysql;
private:
	ProducerConsumerQueue<SQLOperation*>* m_queue;
	std::unique_ptr<DatabaseWorker> m_worker;
	MySQLConnectionInfo& m_connectionInfo;
	ConnectionFlags m_connectionFlags;
	std::mutex m_Mutex;

	MySQLConnection(MySQLConnection const& right) = delete;
	MySQLConnection& operator=(MySQLConnection const& right) = delete;
};

#endif