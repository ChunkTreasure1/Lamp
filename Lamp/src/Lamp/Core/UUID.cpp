#include "lppch.h"
#include "UUID.h"

#include <random>

namespace Lamp
{
	static std::random_device s_RandomDevice;
	static std::mt19937 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint32_t> s_UniformDistribution;

	
	UUID::UUID()
		: m_UUID(s_UniformDistribution(s_RandomEngine))
	{
	}

	UUID::UUID(uint32_t uuid)
		: m_UUID(uuid)
	{
	}

	UUID::UUID(const UUID& other)
		: m_UUID(other.m_UUID)
	{
	}

}