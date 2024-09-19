#ifndef LOGIN_DATABASE_H_
#define LOGIN_DATABASE_H_

#include "MySQLConnection.h"

enum LoginDatabaseStatements : uint32
{
	//{DB}_{SEL/INS/UPD/DEL/REP}_{Summary of data changed}
	LOGIN_SEL_TEST,
    LOGIN_UPD_TEST,
    LOGIN_SEL_LOGONCHALLENGE,
    LOGIN_UPD_LOGONPROOF,
	MAX_LOGINDATABASE_STATEMENTS,
};

class POOL_DATABASE_API LoginDatabaseConnection : public MySQLConnection
{
public:
	typedef LoginDatabaseStatements Statements;

	LoginDatabaseConnection(MySQLConnectionInfo& connInfo);
	LoginDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo);
	~LoginDatabaseConnection() override;

	//load database prepare statements
	void DoPrepareStatements() override;
};

#endif
