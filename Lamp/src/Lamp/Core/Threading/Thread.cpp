#include "lppch.h"
#include "Thread.h"

namespace Lamp
{
	Thread::Thread(const std::string& name, ThreadFunction fn)
		: m_name(name)
	{
		m_thread = std::thread([=]()
			{
				LP_PROFILE_THREAD(m_name.c_str());
				fn();
				m_hasFinished = true;
			});
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
			m_joined = true;
			m_thread.join();
		}
		else
		{
			LP_CORE_WARN("[ThreadPool]: Trying to join already joined thread '{0}'!", m_name);
		}
	}

	const bool Thread::HasFinished() const
	{
		return m_hasFinished.load();
	}
}