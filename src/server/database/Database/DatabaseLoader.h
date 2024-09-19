#ifndef MPOOL_DATABASELOADER_H
#define MPOOL_DATABASELOADER_H

#include "Define.h"
#include <functional>
#include <queue>
#include <stack>
#include <string>

template <class T>
class DatabaseWorkerPool;

class POOL_DATABASE_API DatabaseLoader
{
public:
    DatabaseLoader(const std::string & logger, uint32 const defaultUpdateMask = 0, std::string_view modulesList = {});

    //Register a database to the loader
    template<class T>
    DatabaseLoader& AddDatabase(DatabaseWorkerPool<T>& pool, std::string const& name);

    //load all database
    bool Load();

    enum DatabaseTypeFlags
    {
        DATABASE_NONE       = 0,
        DATABASE_LOGIN      = 1,
        DATABASE_CHARACTER  = 2,
        DATABASE_WORLD      = 4,
        DATABASE_MASK_ALL      = DATABASE_LOGIN | DATABASE_CHARACTER | DATABASE_WORLD,
    };
private:
    bool OpenDatabases();
    bool PrepareStatements();

    using Predicate = std::function<bool()>;
    using Closer = std::function<void()>;

    bool Process(std::queue<Predicate>& queue);

    std::string const _logger;
    std::string_view _modulesList;

    std::queue<Predicate> _open, _prepare;
    std::stack<Closer> _close;
};

#endif //MPOOL_DATABASELOADER_H
