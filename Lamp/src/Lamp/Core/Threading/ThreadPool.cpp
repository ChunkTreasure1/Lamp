#include "lppch.h"
#include "ThreadPool.h"

#include "Lamp/Core/Core.h"
#include "Lamp/Core/Log.h"

namespace Lamp
{
	ThreadPool::ThreadPool()
	{
	}

	ThreadPool::~ThreadPool()
	{
		m_threadPool.clear();
	}

	void ThreadPool::AddThread(const std::string& name, Thread::ThreadFunction function)
	{
		auto it = m_threadPool.find(name);
		if (it != m_threadPool.end())
		{
			LP_CORE_ERROR("Thread with name {0} already exist in pool!");
			LP_CORE_ASSERT(false, "");
		}
		else
		{
			m_threadPool.try_emplace(name, name, function);
		}
	}

	void ThreadPool::JoinAndRemove(const std::string& name)
	{
		auto it = m_threadPool.find(name);
		if (it != m_threadPool.end())
		{
			it->second.Join();
			m_threadPool.erase(name);

			return;
		}

		LP_CORE_ASSERT(false, "Thread name not in pool!");
	}

	void ThreadPool::Join(const std::string& name)
	{
		auto it = m_threadPool.find(name);
		if (it != m_threadPool.end())
		{
			it->second.Join();
			return;
		}

		LP_CORE_ASSERT(false, "Thread name not in pool!");
	}

	void ThreadPool::JoinAll()
	{
		for (auto& thread : m_threadPool)
		{
			thread.second.Join();
		}
	}

	const Thread& ThreadPool::Get(const std::string& name) const
	{
		auto it = m_threadPool.find(name);
		if (it != m_threadPool.end())
		{
			return it->second;
		}

		LP_CORE_ASSERT(false, "Thread name not in pool!");

		return Thread();
	}
}