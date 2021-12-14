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

	~ScopedTimer()
	{
		LP_CORE_INFO("{0} took {1} seconds!", m_name, GetTime());
	}

	inline float GetTime() 
	{
		return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
	};

private:
	std::string m_name;
	std::chrono::high_resolution_clock::time_point m_startTime;
};