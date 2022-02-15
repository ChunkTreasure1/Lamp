#include "lppch.h"
#include "Thread.h"

namespace Lamp
{
	Thread::Thread(const std::string& name, ThreadFunction fn)
		: m_name(name)
	{
		m_thread = std::thread(fn);
	}

	Thread::~Thread()
	{
		if (!m_joined)
		{
			m_thread.join();
		}
	}

	void Thread::Join()
	{
		if (!m_joined)
		{
			LP_CORE_WARN("[ThreadPool]: Trying to join already joined thread '{0}'!", m_name);
			m_joined = true;
			m_thread.join();
		}
	}
}