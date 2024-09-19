#ifndef TRANSATION_H_
#define TRANSATION_H_

#include "DatabaseEnvFwd.h"
#include "Define.h"
#include "SQLOperation.h"
#include "StringFormat.h"

#include <functional>
#include <mutex>
#include <utility>
#include <vector>

class POOL_DATABASE_API TransactionBase
{
	friend class TransactionTask;
	friend class MySQLConnection;

	template<typename T>
	friend class DatabaseWorkerPool;
public:
	TransactionBase() = default;
	virtual ~TransactionBase() { Cleanup(); }

	void Append(std::string_view sql);

	template<typename... Args>
	void Append(std::string_view sql, Args&&... args)
	{
		Append(mpool::StringFormatFmt(sql, std::forward<Args>(args)...));
	}

	[[nodiscard]] std::size_t GetSize() const { return m_queries.size(); }
protected:
	void AppendPreparedStatement(PreparedStatementBase* statement);
	void Cleanup();
	std::vector<SQLElementData> m_queries;

private:
	bool _cleanedUp{ false };
};

template<typename T>
class Transaction : public TransactionBase
{
public:
	using TransactionBase::Append;
	
	void Append(PreparedStatement<T>* statement)
	{
		AppendPreparedStatement(statement);
	}
};

class POOL_DATABASE_API TransactionTask : public SQLOperation
{
	template<typename T>
	friend class DatabaseWorkerPool;

	friend class DatabaseWorker;
	friend class TransactionCallback;

public:
	TransactionTask(std::shared_ptr<TransactionBase> trans) : m_trans(std::move(trans)) {  }
	~TransactionTask() override = default;
protected:
	bool Execute() override;
	int TryExecute();
	void CleanupOnFailure();

	std::shared_ptr<TransactionBase> m_trans;
	static std::mutex _deadlockLock;
};

class POOL_DATABASE_API TransactionWithResultTask : public TransactionTask
{
public:
	TransactionWithResultTask(std::shared_ptr<TransactionBase> trans) : TransactionTask(trans) {}
	TransactionFuture GetFuture() { return m_result.get_future(); }
protected:
	bool Execute() override;

	TransactionPromise m_result;
};

class POOL_DATABASE_API TransactionCallback
{
public:
	TransactionCallback(TransactionFuture&& future) : m_future(std::move(future)) {}
	TransactionCallback(TransactionCallback&&) = default;

	TransactionCallback& operator=(TransactionCallback&&) = default;

	void AfterComplete(std::function<void(bool)> callback) &
	{
		m_callback = std::move(callback);
	}

	bool InvokeIfReady();

	TransactionFuture m_future;
	std::function<void(bool)> m_callback;
};


#endif
