#ifndef WORKERTHREAD_H_
#define WORKERTHREAD_H_

#include "Define.h"

#include <atomic>
#include <thread>


template<typename T>
class ProducerConsumerQueue;

class MySQLConnection;
class SQLOperation;

class POOL_DATABASE_API DatabaseWorker
{
public:
	DatabaseWorker(ProducerConsumerQueue<SQLOperation*>* newQueue, MySQLConnection* connection);
	~DatabaseWorker();
private:
	ProducerConsumerQueue<SQLOperation*>* _queue;
	MySQLConnection* _connection;

	void WorkerThread();
	std::thread _workerThread;

	std::atomic<bool> _cancelationToken;

	DatabaseWorker(DatabaseWorker const& right) = delete;
	DatabaseWorker& operator=(DatabaseWorker const& right) = delete;
};


#endif