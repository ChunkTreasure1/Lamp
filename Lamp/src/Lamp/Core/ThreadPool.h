#pragma once

#include "Lamp/Core/Thread.h"

#include <vector>
#include <thread>
#include <string>
#include <unordered_map>

namespace Lamp
{
	class ThreadPool
	{
	public:

		ThreadPool();
		~ThreadPool();

		void AddThread(const std::string& name, Thread::ThreadFunction function);
		void JoinAndRemove(const std::string& name);
		void Join(const std::string& name);
		void JoinAll();

		const Thread& Get(const std::string& name) const;

	private:
		std::unordered_map<std::string, Thread> m_threadPool;
	};
}