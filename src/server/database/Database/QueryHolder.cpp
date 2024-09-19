#include "QueryHolder.h"
#include "Errors.h"
//#include "Log.h"
#include "MySQLConnection.h"
#include "PreparedStatement.h"
#include "QueryResult.h"

#include <iostream>
#include "StringFormat.h"

bool SQLQueryHolderBase::SetPreparedQueryImpl(size_t index, PreparedStatementBase* stmt)
{
	if (m_queries.size() <= index)
	{
		std::cout << mpool::StringFormatFmt("Query index ({}) out of range (size:{}) for prepared statement",
			uint32(index), (uint32)m_queries.size()) << std::endl;
		return false;
	}

	m_queries[index].first = stmt;
	return true;
}

PreparedQueryResult SQLQueryHolderBase::GetPreparedResult(size_t index) const
{
	ASSERT(index < m_queries.size(), "Query holder result index out of range, tried to access index {} but there are only {} results");
	
	return m_queries[index].second;
}

void SQLQueryHolderBase::SetPreparedResult(size_t index, PreparedResultSet* result)
{
	if (result && !result->GetRowCount())
	{
		delete result;
		result = nullptr;
	}

	if (index < m_queries.size())
		m_queries[index].second = PreparedQueryResult(result);
}

SQLQueryHolderBase::~SQLQueryHolderBase()
{
	for (std::pair<PreparedStatementBase*, PreparedQueryResult>& query : m_queries)
	{
		delete query.first;
	}
}

void SQLQueryHolderBase::SetSize(size_t size)
{
	m_queries.resize(size);
}

SQLQueryHolderTask::~SQLQueryHolderTask() = default;

bool SQLQueryHolderTask::Execute()
{
	for (size_t i = 0; i < m_holder->m_queries.size(); i++)
		if (PreparedStatementBase* stmt = m_holder->m_queries[i].first)
			m_holder->SetPreparedResult(i, m_conn->Query(stmt));
	m_result.set_value();
	return true;
}

bool SQLQueryHolderCallback::InvokeIfReady()
{
	if (m_future.valid() && m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		m_callback(*m_holder);
		return true;
	}

	return false;
}