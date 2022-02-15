#include "lppch.h"
#include "ShaderStorageBufferSet.h"

namespace Lamp
{
	ShaderStorageBufferSet::ShaderStorageBufferSet(uint32_t count)
		: m_count(count)
	{
	}

	void ShaderStorageBufferSet::Add(uint32_t dataSize, uint32_t binding, uint32_t set)
	{
		for (uint32_t i = 0; i < m_count; i++)
		{
			m_shaderStorageBuffers[i][set][binding] = ShaderStorageBuffer::Create(dataSize);
		}
	}

	Ref<ShaderStorageBuffer> ShaderStorageBufferSet::Get(uint32_t binding, uint32_t set, uint32_t index)
	{
		return m_shaderStorageBuffers[index][set][binding];
	}

	Ref<ShaderStorageBufferSet> ShaderStorageBufferSet::Create(uint32_t count)
	{
		return CreateRef<ShaderStorageBufferSet>(count);
	}
}