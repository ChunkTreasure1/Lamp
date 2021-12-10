#pragma once

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

		void AddThread(const std::string& name, Ref<std::thread> thread);
		void JoinAndRemove(const std::string& name);
		void Join(const std::string& name);

		const Ref<std::thread> Get(const std::string& name) const;

	private:
		std::unordered_map<std::string, Ref<std::thread>> m_threadPool;
	};
}