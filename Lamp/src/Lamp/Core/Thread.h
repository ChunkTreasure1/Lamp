#pragma once

#include "Lamp/Core/Time/FrameTime.h"

namespace Lamp
{
	class Thread
	{
	public:
		using ThreadFunction = std::function<void()>;

		Thread(const std::string& name, ThreadFunction fn);
		Thread() = default;
		~Thread();

		void Join();

		inline const std::string& GetName() { return m_name; }

	private:
		bool m_joined = false;
		std::thread m_thread;
		std::string m_name;
	};
}