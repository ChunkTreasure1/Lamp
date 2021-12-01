#include "lppch.h"
#include "UniformBufferSet.h"

namespace Lamp
{
	UniformBufferSet::UniformBufferSet(uint32_t count)
		: m_count(count)
	{
	}

	UniformBufferSet::~UniformBufferSet()
	{
	}

	void UniformBufferSet::Add(const void* pData, uint32_t dataSize, uint32_t binding, uint32_t set)
	{
		for (uint32_t i = 0; i < m_count; i++)
		{
			m_uniformBuffers[i][set][binding] = UniformBuffer::Create(pData, dataSize);
		}
	}

	Ref<UniformBuffer> UniformBufferSet::Get(uint32_t binding, uint32_t set, uint32_t index)
	{
		return m_uniformBuffers[index][set][binding];
	}

	Ref<UniformBufferSet> UniformBufferSet::Create(uint32_t count)
	{
		return CreateRef<UniformBufferSet>(count);
	}
}