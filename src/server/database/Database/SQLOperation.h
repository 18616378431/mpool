#ifndef SQLOPERATION_H_
#define SQLOPERATION_H_

#include "DatabaseEnvFwd.h"
#include "Define.h"

#include <variant>

enum SQLElementDataType
{
	SQL_ELEMENT_RAW,
	SQL_ELEMENT_PREPARED
};

struct SQLElementData
{
	std::variant<PreparedStatementBase*, std::string> element;
	SQLElementDataType type;
};

class MySQLConnection;

class POOL_DATABASE_API SQLOperation
{
public:
	SQLOperation() = default;
	virtual ~SQLOperation() = default;

	virtual int call()
	{
		Execute();
		return 0;
	}

	virtual bool Execute() = 0;
	virtual void SetConnection(MySQLConnection* con)
	{
		m_conn = con;
	}

	MySQLConnection* m_conn{nullptr};
private:
	SQLOperation(SQLOperation const& right) = delete;
	SQLOperation& operator=(SQLOperation const& right) = delete;
};

#endif // !SQLOPERATION_H_
