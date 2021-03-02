#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue
{
public:
	ThreadSafeQueue() {}
	ThreadSafeQueue(const ThreadSafeQueue& copy)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_DataQueue = copy.m_DataQueue;
	}

	void Push(T val)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_DataQueue.push(val);
		m_DataCondVar.notify_one();
	}

	void WaitAndPop(T& val)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_DataCondVar.wait([&m_DataQueue]() { return !m_DataQueue.empty(); });

		val = m_DataQueue.front();
		m_DataQueue.pop();
	}

	std::shared_ptr<T> WaitAndPop()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_DataCondVar.wait([&m_DataQueue]() { return !m_DataQueue.empty(); });

		std::shared_ptr<T> val(std::make_shared<T>(m_DataQueue.front()));
		m_DataQueue.pop();

		return val;
	}

	bool TryPop(T& val)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (m_DataQueue.empty())
		{
			return false;
		}

		val = m_DataQueue.front();
		m_DataQueue.pop();

		return true;
	}

	std::shared_ptr<T> TryPop()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (m_DataQueue.empty())
		{
			return std::shared_ptr<T>();
		}

		std::shared_ptr<T> val(std::make_shared<T>(m_DataQueue.front()));
		m_DataQueue.pop();

		return val;
	}

	bool Empty()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_DataQueue.empty();
	}

	uint32_t Size()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		return static_cast<uint32_t>(m_DataQueue.size());
	}

private:
	std::mutex m_Mutex;
	std::queue<T> m_DataQueue;
	std::condition_variable m_DataCondVar;
};
