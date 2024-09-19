#include "MySQLPreparedStatement.h"
#include "Errors.h"
//#include "Log.h"
#include "MySQLHacks.h"
#include "PreparedStatement.h"

#include "StringFormat.h"

#include <iostream>

template<typename T>
struct MySQLType {};

template<> struct MySQLType<uint8> : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY> {};
template<> struct MySQLType<uint16> : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT> {};
template<> struct MySQLType<uint32> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG> {};
template<> struct MySQLType<uint64> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG> {};
template<> struct MySQLType<int8> : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY> {};
template<> struct MySQLType<int16> : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT> {};
template<> struct MySQLType<int32> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG> {};
template<> struct MySQLType<int64> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG> {};
template<> struct MySQLType<float> : std::integral_constant<enum_field_types, MYSQL_TYPE_FLOAT> {};
template<> struct MySQLType<double> : std::integral_constant<enum_field_types, MYSQL_TYPE_DOUBLE> {};

MySQLPreparedStatement::MySQLPreparedStatement(MySQLStmt* stmt, std::string_view queryString):
	m_stmt(nullptr),
	m_Mstmt(stmt),
	m_bind(nullptr),
	m_queryString(std::string(queryString))
{
	m_paramCount = mysql_stmt_param_count(stmt);
	m_paramsSet.assign(m_paramCount, false);
	m_bind = new MySQLBind[m_paramCount];
	memset(m_bind, 0, sizeof(MySQLBind) * m_paramCount);

	MySQLBool bool_tmp = MySQLBool(1);
	mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &bool_tmp);
}

MySQLPreparedStatement::~MySQLPreparedStatement()
{
	ClearParameters();
	if (m_Mstmt->bind_result_done)
	{
		delete[] m_Mstmt->bind->length;
		delete[] m_Mstmt->bind->is_null;
	}

	mysql_stmt_close(m_Mstmt);
	delete[] m_bind;
}

void MySQLPreparedStatement::BindParameters(PreparedStatementBase* stmt)
{
	m_stmt = stmt;

	uint8 pos = 0;
	for (PreparedStatementData const& data : stmt->GetParameters())
	{
		std::visit([&](auto&& param)
			{
				SetParameter(pos, param);
			}, data.data);
		++pos;
	}
#ifdef _DEBUG
	if (pos < m_paramCount)
		std::cout << mpool::StringFormatFmt("BindParameters() for statement {} did not binf all allocated parameters",
			stmt->GetIndex()) << std::endl;
#endif
}

void MySQLPreparedStatement::ClearParameters()
{
	for (uint32 i = 0; i < m_paramCount; ++i)
	{
		delete m_bind[i].length;
		m_bind[i].length = nullptr;
		delete[] (char*)m_bind[i].buffer;
		m_bind[i].buffer = nullptr;
		m_paramsSet[i] = false;
	}
}

static bool ParamenterIndexAssertFail(uint32 stmtIndex, uint8 index, uint32 paramCount)
{
	std::cout << mpool::StringFormatFmt("Attempted to bind parameter {}{} on a PreparedStatement {} (statement has only {} parameters)",
		uint32(index) + 1, (index == 1 ? "st" : (index == 2 ? "nd" : (index == 3 ? "rd" : "nd"))), stmtIndex, paramCount) << std::endl;
	
	return false;
}

void MySQLPreparedStatement::AssertValidIndex(const uint8 index)
{
	ASSERT(index < m_paramCount || ParamenterIndexAssertFail(m_stmt->GetIndex(), index, m_paramCount));

	if (m_paramsSet[index])
		std::cout << mpool::StringFormatFmt("PreparedStatement (id:{}) trying to bind value on already bound index({})",
			m_stmt->GetIndex(), index) << std::endl;
}

template<typename T>
void MySQLPreparedStatement::SetParameter(const uint8 index, T value)
{
	AssertValidIndex(index);
	m_paramsSet[index] = true;
	MYSQL_BIND* param = &m_bind[index];
	uint32 len = uint32(sizeof(T));
	param->buffer_type = MySQLType<T>::value;
	delete[] static_cast<char*>(param->buffer);
	param->buffer = new char[len];
	param->buffer_length = 0;
	param->is_null_value = 0;
	param->length = nullptr;
	param->is_unsigned = std::is_unsigned_v<T>;

	memcpy(param->buffer, &value, len);
}

void MySQLPreparedStatement::SetParameter(const uint8 index, bool value)
{
	SetParameter(index, uint8(value ? 1 : 0));
}

void MySQLPreparedStatement::SetParameter(const uint8 index, std::nullptr_t value)
{
	AssertValidIndex(index);
	m_paramsSet[index] = true;
	MYSQL_BIND* param = &m_bind[index];
	param->buffer_type = MYSQL_TYPE_NULL;
	delete[] static_cast<char*>(param->buffer);
	param->buffer = nullptr;
	param->buffer_length = 0;
	param->is_null_value = 1;
	delete param->length;
	param->length = nullptr;
}

void MySQLPreparedStatement::SetParameter(const uint8 index, std::string const& value)
{
	AssertValidIndex(index);
	m_paramsSet[index] = true;
	MYSQL_BIND* param = &m_bind[index];
	uint32 len = uint32(value.size());
	param->buffer_type = MYSQL_TYPE_VAR_STRING;
	delete[] static_cast<char*>(param->buffer);
	param->buffer = new char[len];
	param->buffer_length = len;
	param->is_null_value = 0;
	delete param->length;
	param->length = new unsigned long(len);

	memcpy(param->buffer, value.c_str(), len);
}

void MySQLPreparedStatement::SetParameter(const uint8 index, std::vector<uint8> const& value)
{
	AssertValidIndex(index);
	m_paramsSet[index] = true;
	MYSQL_BIND* param = &m_bind[index];
	uint32 len = uint32(value.size());
	param->buffer_type = MYSQL_TYPE_BLOB;
	delete[] static_cast<char*>(param->buffer);
	param->buffer = new char[len];
	param->buffer_length = len;
	param->is_null_value = 0;
	delete param->length;
	param->length = new unsigned long(len);

	memcpy(param->buffer, value.data(), len);
}

std::string MySQLPreparedStatement::getQueryString() const
{
	std::string queryString(m_queryString);

	size_t pos = 0;

	for (PreparedStatementData const& data : m_stmt->GetParameters())
	{
		pos = queryString.find('?', pos);

		std::string replaceStr = std::visit([&](auto&& data) 
		{
			return PreparedStatementData::ToString(data);
		}, data.data);

		queryString.replace(pos, 1, replaceStr);
		pos += replaceStr.length();
	}

	return queryString;
}