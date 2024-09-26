#include "LoginDatabase.h"
#include "MySQLPreparedStatement.h"

void LoginDatabaseConnection::DoPrepareStatements()
{
	if (!m_reconnecting)
		m_stmts.resize(MAX_LOGINDATABASE_STATEMENTS);

//	PrepareStatement(LOGIN_SEL_TEST,
//                     "select name from test where id = ?", CONNECTION_SYNCH);
//	PrepareStatement(LOGIN_UPD_TEST,
//                     "update test set count = count + 1 where id = ?", CONNECTION_ASYNC);
    PrepareStatement(LOGIN_SEL_LOGONCHALLENGE,
                     "SELECT id, username, last_ip, salt, verifier FROM account a WHERE username = ? limit 1",
                     CONNECTION_SYNCH);
    PrepareStatement(LOGIN_UPD_LOGONPROOF, "UPDATE account SET session_key = ?, last_ip = ?, last_login = NOW(), locale = ?, os = ? WHERE username = ?",
                     CONNECTION_SYNCH);
}

LoginDatabaseConnection::LoginDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo)
{

}

LoginDatabaseConnection::LoginDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo)
{

}

LoginDatabaseConnection::~LoginDatabaseConnection()
{

}