#pragma once

#include "Lamp/Core/Log.h"

#include <chrono>
#include <string>

class ScopedTimer
{
public:
	ScopedTimer(const std::string& name)
		: m_name(name)
	{ 
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	ScopedTimer()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	~ScopedTimer()
	{
		if (!m_name.empty())
		{
			LP_CORE_INFO("{0} finished in {1} ms!", m_name, GetTime());
		}
	}

	inline float GetTime()
	{
		return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
	};

private:
	std::string m_name;
	std::chrono::high_resolution_clock::time_point m_startTime;
};