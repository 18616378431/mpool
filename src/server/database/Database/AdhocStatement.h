#ifndef ADHOCSTATEMENT_H_
#define ADHOCSTATEMENT_H_

#include "DatabaseEnvFwd.h"
#include "Define.h"
#include "SQLOperation.h"

class POOL_DATABASE_API BasicStatementTask : public SQLOperation
{
public:
	BasicStatementTask(std::string_view sql, bool async = false);
	~BasicStatementTask();

	bool Execute() override;
	QueryResultFuture GetFuture() const { return m_result->get_future(); }
private:
	std::string m_sql;
	bool m_has_result;
	QueryResultPromise* m_result;
};

#endif
