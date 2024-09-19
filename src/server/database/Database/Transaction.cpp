#include "Transaction.h"
#include "Errors.h"
//#include "Log.h"
#include "MySQLConnection.h"
#include "PreparedStatement.h"
#include "Timer.h"

#include <mysqld_error.h>
#include <sstream>
#include <thread>

#include <iostream>
#include "StringFormat.h"

std::mutex TransactionTask::_deadlockLock;

constexpr Milliseconds DEADLOCK_MAX_RETRY_TIME_MS = 1min;

void TransactionBase::Append(std::string_view sql)
{
	SQLElementData data = {};
	data.type = SQL_ELEMENT_RAW;
	data.element = std::string(sql);
	m_queries.emplace_back(data);
}

void TransactionBase::AppendPreparedStatement(PreparedStatementBase* stmt)
{
	SQLElementData data = {};
	data.type = SQL_ELEMENT_PREPARED;
	data.element = stmt;
	m_queries.emplace_back(data);
}

void TransactionBase::Cleanup()
{
	if (_cleanedUp)
		return ;

	for (SQLElementData& data : m_queries)
	{
		switch (data.type)
		{
		case SQL_ELEMENT_PREPARED:
		{
			try
			{
				PreparedStatementBase* stmt = std::get<PreparedStatementBase*>(data.element);
				ASSERT(stmt);
				delete stmt;
			}
			catch (const std::bad_variant_access& ex)
			{
				//ABORT();
				std::cout << mpool::StringFormatFmt("PreparedStatementBase not found in SQLElementData.{}",
					ex.what()) << std::endl;
				return;
			}
		}
			break;
		case SQL_ELEMENT_RAW:
		{
			try
			{
				std::get<std::string>(data.element).clear();
			}
			catch (const std::bad_variant_access& ex)
			{
				//ABORT();
				std::cout << mpool::StringFormatFmt("std::string not found in SQLElementData.{}",
					ex.what()) << std::endl;
				return;
			}
		}
			break;
		}
	}

	m_queries.clear();
	_cleanedUp = true;
}

bool TransactionTask::Execute()
{
	int errorCode = TryExecute();

	if (!errorCode)
		return true;

	if (errorCode == ER_LOCK_DEADLOCK)
	{
		std::ostringstream threadIdStream;
		threadIdStream << std::this_thread::get_id();
		std::string threadId = threadIdStream.str();

		{
			std::lock_guard<std::mutex> lock(_deadlockLock);

			for (Milliseconds loopDuration = 0s, startMSTime = GetTimeMS(); loopDuration <= DEADLOCK_MAX_RETRY_TIME_MS; loopDuration = GetMSTimeDiffToNow(startMSTime))
			{
				if (!TryExecute())
					return true;

				std::cout << mpool::StringFormatFmt("Deadlocked SQL Transaction, retrying.Loop timer : {} ms,Thread Id: {}",
					loopDuration.count(), threadId) << std::endl;
			}
		}

		std::cout << mpool::StringFormatFmt("Fatal deadlocked SQL Transaction, it will not be retried anymore. ThreadId : {}",
			threadId) << std::endl;
	}

	CleanupOnFailure();

	return false;
}

int TransactionTask::TryExecute()
{
	return m_conn->ExecuteTransaction(m_trans);
	return 0;
}

void TransactionTask::CleanupOnFailure()
{
	m_trans->Cleanup();
}

bool TransactionWithResultTask::Execute()
{
	int errorCode = TryExecute();

	if (!errorCode)
	{
		m_result.set_value(true);
		return true;
	}

	if (errorCode == ER_LOCK_DEADLOCK)
	{
		std::ostringstream threadIdStream;
		threadIdStream << std::this_thread::get_id();
		std::string threadId = threadIdStream.str();

		{
			std::lock_guard<std::mutex> lock(_deadlockLock);

			for (Milliseconds loopDuration = 0s, startMSTime = GetTimeMS(); loopDuration <= DEADLOCK_MAX_RETRY_TIME_MS; loopDuration = GetMSTimeDiffToNow(startMSTime))
			{
				if (!TryExecute())
				{
					m_result.set_value(true);
					return true;
				}

				std::cout << mpool::StringFormatFmt("Deadlocked SQL Transaction, retrying.Loop timer : {} ms,Thread Id: {}",
					loopDuration.count(), threadId) << std::endl;
			}
		}

		std::cout << mpool::StringFormatFmt("Fatal deadlocked SQL Transaction, it will not be retried anymore. ThreadId : {}",
			threadId) << std::endl;
	}

	CleanupOnFailure();
	m_result.set_value(false);

	return false;
}

bool TransactionCallback::InvokeIfReady()
{
	if (m_future.valid() && m_future.wait_for(0s) == std::future_status::ready)
	{
		m_callback(m_future.get());
		return true;
	}

	return false;
}

