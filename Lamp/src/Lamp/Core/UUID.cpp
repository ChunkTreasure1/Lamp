#include "lppch.h"
#include "UUID.h"

#include <random>

namespace Lamp
{
	static std::random_device s_RandomDevice;
	static std::mt19937 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution64;

	static std::uniform_int_distribution<int> s_UniformDistribution;

	
	UUID::UUID()
		: m_UUID(s_UniformDistribution64(s_RandomEngine))
	{
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
	}

	UUID::UUID(const UUID& other)
		: m_UUID(other.m_UUID)
	{
	}

	GraphUUID::GraphUUID()
		: m_UUID(s_UniformDistribution(s_RandomEngine))
	{
	}

	GraphUUID::GraphUUID(int uuid)
		: m_UUID(uuid)
	{
	}

	GraphUUID::GraphUUID(const GraphUUID& other)
		: m_UUID(other.m_UUID)
	{
	}

}