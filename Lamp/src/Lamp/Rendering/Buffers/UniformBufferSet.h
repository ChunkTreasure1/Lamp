#pragma once

#include "Lamp/Rendering/Buffers/UniformBuffer.h"

#include <map>

namespace Lamp
{
	class UniformBufferSet
	{
	public:
		UniformBufferSet(uint32_t count);
		~UniformBufferSet() = default;

		void Add(const void* pData, uint32_t dataSize, uint32_t binding, uint32_t set);
		Ref<UniformBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t index = 0);

		static Ref<UniformBufferSet> Create(uint32_t count);

	private:
		uint32_t m_count;
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<UniformBuffer>>>> m_uniformBuffers;
	};
}