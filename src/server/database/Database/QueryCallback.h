#ifndef QUERYCALLBACK_H_
#define QUERYCALLBACK_H_

#include "DatabaseEnvFwd.h"
#include "Define.h"

#include <functional>
#include <future>
#include <list>
#include <queue>
#include <utility>


class POOL_DATABASE_API QueryCallback
{
public:
	explicit QueryCallback(QueryResultFuture&& result);
	explicit QueryCallback(PreparedQueryResultFuture && result);

	QueryCallback(QueryCallback&& right) noexcept;
	QueryCallback& operator=(QueryCallback&& right) noexcept;
	~QueryCallback();

	QueryCallback&& WithCallback(std::function<void(QueryResult)>&& callback);
	QueryCallback&& WithPreparedCallback(std::function<void(PreparedQueryResult)>&& callback);

	QueryCallback&& WithChainingCallback(std::function<void(QueryCallback&, QueryResult)>&& callback);
	QueryCallback&& WithChainingPreparedCallback(std::function<void(QueryCallback&, PreparedQueryResult)>&& callback);

	void SetNextQuery(QueryCallback&& next);

	bool InvokeIfReady();
private:
	QueryCallback(QueryCallback const& right) = delete;
	QueryCallback& operator=(QueryCallback const& right) = delete;

	template<typename T> friend void ConstructActiveMember(T* obj);
	template<typename T> friend void DestroyActiveMember(T* obj);
	template<typename T> friend void MoveFrom(T* to, T&& from);

	union 
	{
		QueryResultFuture _string;
		PreparedQueryResultFuture _prepared;
	};

	bool _isPrepared;

	struct QueryCallbackData;
	std::queue<QueryCallbackData, std::list<QueryCallbackData>> _callbacks;
};

#endif