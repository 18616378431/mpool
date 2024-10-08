#ifndef PCQUEUE_H_
#define PCQUEUE_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <type_traits>

template<typename T>
class ProducerConsumerQueue
{
private:
	std::mutex _queueLock;
	std::queue<T> _queue;
	std::condition_variable _condition;
	std::atomic<bool> _shutdown;
public:
	ProducerConsumerQueue() : _shutdown(false) { }

	void Push(const T& value)
	{
		std::lock_guard<std::mutex> lock(_queueLock);
		_queue.push(std::move(value));
		
		_condition.notify_one();
	}

	bool Empty()
	{
		std::lock_guard<std::mutex> lock(_queueLock);

		return _queue.empty();
	}

	[[nodiscard]] size_t Size() const
	{
		return _queue.size();
	}

	bool Pop(T& value)
	{
		std::lock_guard<std::mutex> lock(_queueLock);

		if (_queue.empty() || _shutdown)
		{
			return false;
		}

		value = _queue.front();
		
		_queue.pop();

		return true;
	}

	void WaitAndPop(T& value)
	{
		std::unique_lock<std::mutex> lock(_queueLock);

		while (_queue.empty() && !_shutdown)
		{
			_condition.wait(lock);
		}

		if (_queue.empty() || _shutdown)
		{
			return;
		}

		value = _queue.front();

		_queue.pop();
	}

	void Cancel()
	{
		std::unique_lock<std::mutex> lock(_queueLock);

		while (!_queue.empty())
		{
			T& value = _queue.front();

			DeleteQueuedObject(value);

			_queue.pop();
		}

		_shutdown = true;

		_condition.notify_all();
	}

private:
	template<typename E = T>
	typename std::enable_if<std::is_pointer<E>::value>::type DeleteQueuedObject(E& obj) { delete obj; }

	template<typename E = T>
	typename std::enable_if<!std::is_pointer<E>::value>::type DeleteQueuedObject(E const&) { }
};



#endif