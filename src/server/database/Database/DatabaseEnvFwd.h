#ifndef DATABASE_ENV_FWD_H_
#define DATABASE_ENV_FWD_H_

#include <future>
#include <memory>

#include "MySQLHacks.h"

struct QueryResultFieldMetadata;
class Field;

class ResultSet;
using QueryResult = std::shared_ptr<ResultSet>;
using QueryResultFuture = std::future<QueryResult>;
using QueryResultPromise = std::promise<QueryResult>;

class LoginDatabaseConnection;

class PreparedStatementBase;

template<typename T>
class PreparedStatement;

using LoginDatabasePreparedStatement = PreparedStatement<LoginDatabaseConnection>;

class PreparedResultSet;
using PreparedQueryResult = std::shared_ptr<PreparedResultSet>;
using PreparedQueryResultFuture = std::future<PreparedQueryResult>;
using PreparedQueryResultPromise = std::promise<PreparedQueryResult>;

class QueryCallback;

template<typename T>
class AsyncCallbackProcessor;

using QueryCallbackProcessor = AsyncCallbackProcessor<QueryCallback>;

class TransactionBase;

using TransactionFuture = std::future<bool>;
using TransactionPromise = std::promise<bool>;

template<typename T>
class Transaction;

class TransactionCallback;

template<typename T>
using SQLTransaction = std::shared_ptr<Transaction<T> >;

using LoginDatabaseTransaction = SQLTransaction<LoginDatabaseConnection>;

class SQLQueryHolderBase;
using QueryResultHolderFuture = std::future<void>;
using QueryResultHolderPromise = std::promise<void>;

template<typename T>
class SQLQueryHolder;

using LoginDatabaseQueryHolder = SQLQueryHolder<LoginDatabaseConnection>;

class SQLQueryHolderCallback;

struct MySQLHandle;
struct MySQLResult;
struct MySQLField;
struct MySQLBind;
struct MySQLStmt;

#endif