#ifndef MPOOL_DATABASEENV_H
#define MPOOL_DATABASEENV_H

#include "DatabaseWorkerPool.h"
#include "Define.h"

#include "Implementation/LoginDatabase.h"

#include "Field.h"
#include "PreparedStatement.h"
#include "QueryCallback.h"
#include "QueryResult.h"
#include "Transaction.h"

POOL_DATABASE_API extern DatabaseWorkerPool<LoginDatabaseConnection> LoginDatabase;

#endif //MPOOL_DATABASEENV_H
