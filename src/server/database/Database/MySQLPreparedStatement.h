#ifndef MYSQLPREPAREDSTATEMENT_H_
#define MYSQLPREPAREDSTATEMENT_H_

#include <vector>

#include "DatabaseEnvFwd.h"
#include "Define.h"
#include "MySQLWorkaround.h"

class MySQLConnection;
class PreparedStatementBase;

class POOL_DATABASE_API MySQLPreparedStatement
{
	friend class MySQLConnection;
	friend class PreparedStatementBase;

public:
	MySQLPreparedStatement(MySQLStmt* stmt, std::string_view queryString);
	~MySQLPreparedStatement();

	void BindParameters(PreparedStatementBase* stmt);

	uint32 GetParameterCount() const { return m_paramCount; }
protected:
	void SetParameter(const uint8 index, bool value);
	void SetParameter(const uint8 index, std::nullptr_t value);
	void SetParameter(const uint8 index, std::string const& value);
	void SetParameter(const uint8 index, std::vector<uint8> const& value);

	template<typename T>
	void SetParameter(const uint8 index, T value);

	MySQLStmt* GetSTMT() { return m_Mstmt; }
	MySQLBind* GetBind() { return m_bind; }
	PreparedStatementBase* m_stmt;
	void ClearParameters();
	void AssertValidIndex(const uint8 index);
	std::string getQueryString() const;
private:
	MySQLStmt* m_Mstmt;
	uint32 m_paramCount;
	std::vector<bool> m_paramsSet;
	MySQLBind* m_bind;
	std::string m_queryString{};

	MySQLPreparedStatement(MySQLPreparedStatement const& right) = delete;
	MySQLPreparedStatement& operator=(MySQLPreparedStatement const& right) = delete;
};

#endif
