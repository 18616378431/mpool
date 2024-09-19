#include "PreparedStatement.h"
#include "Errors.h"
//#include "Log.h"
#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"
#include "MySQLWorkaround.h"
#include "QueryResult.h"

#include "StringFormat.h"

#include <iostream>

PreparedStatementBase::PreparedStatementBase(uint32 index, uint8 capacity) : 
	m_index(index), 
	statement_data(capacity)
{

}

PreparedStatementBase::PreparedStatementBase(std::shared_ptr<MySQLPreparedStatement> preparedStatement, uint8 capacity) :
        m_preparedStatement(preparedStatement),
        statement_data(capacity)
{

}

PreparedStatementBase::~PreparedStatementBase()
{

}

//bind to buffer
template<typename T>
mpool::Types::is_non_string_view_v<T> PreparedStatementBase::SetValidData(const uint8 index, T const& value)
{
	ASSERT(index < statement_data.size());
	statement_data[index].data.emplace<T>(value);
}

//none template func
void PreparedStatementBase::SetValidData(const uint8 index)
{
	ASSERT(index < statement_data.size());
	statement_data[index].data.emplace<std::nullptr_t>(nullptr);
}

void PreparedStatementBase::SetValidData(const uint8 index, std::string_view value)
{
	ASSERT(index < statement_data.size());
	statement_data[index].data.emplace<std::string>(value);
}

template void PreparedStatementBase::SetValidData(const uint8 index, uint8 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, int8 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, uint16 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, int16 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, uint32 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, int32 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, uint64 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, int64 const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, bool const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, float const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, std::string const& value);
template void PreparedStatementBase::SetValidData(const uint8 index, std::vector<uint8> const& value);

//execution
PreparedStatementTask::PreparedStatementTask(PreparedStatementBase* stmt, bool async) :	
	m_stmt(stmt),
	m_result(nullptr)
{
	m_has_result = async;

	if (async) 
	{
		m_result = new PreparedQueryResultPromise();
	}
}

PreparedStatementTask::~PreparedStatementTask()
{
	delete m_stmt;

	if (m_has_result && m_result)
	{
		delete m_result;
	}
}

bool PreparedStatementTask::Execute()
{
    LoginDatabasePreparedStatement* stmt_upd_just = nullptr;

    if (m_stmt->isPrepared)
    {
        std::shared_ptr<MySQLPreparedStatement> mysql_stmt = m_conn->PrepareStatement(m_stmt->sql, CONNECTION_ASYNC);
        uint32 const paramCount = mysql_stmt->GetParameterCount();

        ASSERT(paramCount < std::numeric_limits<uint8>::max());

        uint8 _preparedStatementParamsCount = static_cast<uint8>(paramCount);

        stmt_upd_just = new PreparedStatement<LoginDatabaseConnection>((LoginDatabaseConnection *)m_conn, mysql_stmt, _preparedStatementParamsCount);

        stmt_upd_just->SetParameters(m_stmt->GetParameters());
    }

    //result query
	if (m_has_result)
	{
        PreparedResultSet* result = nullptr;

        if (m_stmt->isPrepared)
        {
            stmt_upd_just->sql = "";
            stmt_upd_just->isPrepared = m_stmt->isPrepared;

            result = m_conn->Query(stmt_upd_just);

            delete stmt_upd_just;
        }
        else
        {
            result = m_conn->Query(m_stmt);
        }

		if (!result || !result->GetRowCount())
		{
			delete result;
			m_result->set_value(PreparedQueryResult(nullptr));
			return false;
		}

		m_result->set_value(PreparedQueryResult(result));
		return true;
	}

    //exe
    if (m_stmt->isPrepared)
    {
        stmt_upd_just->sql = "";
        stmt_upd_just->isPrepared = m_stmt->isPrepared;

        bool result = m_conn->Execute(stmt_upd_just);

        delete stmt_upd_just;

        return result;
    }
    else
    {
        return m_conn->Execute(m_stmt);
    }

	return true;
}

template<typename T>
std::string PreparedStatementData::ToString(T value)
{
	return mpool::StringFormatFmt("{}", value);
}

template<>
std::string PreparedStatementData::ToString(std::vector<uint8>)
{
	return "BINARY";
}

template std::string PreparedStatementData::ToString(uint8);
template std::string PreparedStatementData::ToString(uint16);
template std::string PreparedStatementData::ToString(uint32);
template std::string PreparedStatementData::ToString(uint64);
template std::string PreparedStatementData::ToString(int8);
template std::string PreparedStatementData::ToString(int16);
template std::string PreparedStatementData::ToString(int32);
template std::string PreparedStatementData::ToString(int64);
template std::string PreparedStatementData::ToString(std::string);
template std::string PreparedStatementData::ToString(float);
template std::string PreparedStatementData::ToString(double);
template std::string PreparedStatementData::ToString(bool);

std::string PreparedStatementData::ToString(std::nullptr_t)
{
	return "NULL";
}