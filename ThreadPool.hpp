#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>
#include <iostream>
#include "Console.hpp"
#include "ProcessMonitor.cpp"

class Task
{
public:
	std::string destination;
	std::string source;
	int flag;
	int fd;
};

template <typename T = Task>
class Channel
{
private:
	std::queue<T> queue;
	std::mutex mutex;
	std::condition_variable not_empty;
	bool isTerminated;
	static const int MAX_ITEM_QUEUED = 10000;

public:
	Channel()
	{
		// Initialize queue
		this->queue = std::queue<T>();
		this->isTerminated = false;
	}

	void send(const T &value)
	{
		// Get exclusive access to the queue
		std::unique_lock<std::mutex> l(this->mutex);
		if (this->queue.size() >= MAX_ITEM_QUEUED)
			return;

		// Put value into queue
		this->queue.push(value);

		// Notify receivers
		this->not_empty.notify_one();
	}

	void sendStop()
	{
		this->isTerminated = true;
		Console::printInfo("All the thread were ordered to die.");
		this->not_empty.notify_all();
	}

	bool recv(T &value)
	{
		// Get exclusive access to the queue when it's not empty
		std::unique_lock<std::mutex> l(this->mutex);
		this->not_empty.wait(l, [this]
							 { return !this->queue.empty() || this->isTerminated; });
		if (this->isTerminated)
		{
			return false;
		}
		else
		{
			value = this->queue.front();
			this->queue.pop();
			return true;
		}
	}
};
template <typename T = Task>
class Worker
{
private:
	std::shared_ptr<Channel<T>> chan;
	std::thread thread;
	uint threadID;
	static uint runningThread;

public:
	Worker(std::shared_ptr<Channel<T>> &chan, void (*asyncFunction)(T task))
	{
		this->chan = chan;
		this->threadID = runningThread + 1;
		this->runningThread += 1;
		this->thread = std::thread([=]()
								   {
			for (;;)
			{
				T value;
				if(!chan->recv(value))
				{
					break;
				}
				ProcessMonitor pm("Worker ");
				asyncFunction(value);
			} });
	}

	~Worker()
	{
		this->runningThread -= 1;
		if (this->thread.joinable())
		{
			this->thread.join();
		}
		if (this->runningThread == 0)
		{
			Console::printInfo("All workers are destroyed.");
		}
	}
};
template <typename T>
uint Worker<T>::runningThread = 0;

template <typename T, int N>
class ThreadPool
{

private:
	std::array<std::unique_ptr<Worker<T>>, N> pool;
	std::shared_ptr<Channel<T>> chan;

public:
	ThreadPool(void (*asyncFunction)(T task))
	{
		this->chan = std::make_shared<Channel<T>>();
		for (auto &worker : this->pool)
		{
			worker = std::make_unique<Worker<T>>(chan, asyncFunction);
		}
	}
	~ThreadPool()
	{
		this->chan->sendStop();
	}

	void send(const T &task)
	{
		this->chan->send(task);
	}
};
