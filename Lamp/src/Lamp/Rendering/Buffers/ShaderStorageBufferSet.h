#pragma once

#include "Lamp/Rendering/Buffers/ShaderStorageBuffer.h"

#include <map>

namespace Lamp
{
	class ShaderStorageBufferSet
	{
	public:
		ShaderStorageBufferSet(uint32_t count);
		~ShaderStorageBufferSet() = default;

		void Add(uint32_t dataSize, uint32_t binding, uint32_t set);
		Ref<ShaderStorageBuffer> Get(uint32_t binding, uint32_t set = 0, uint32_t index = 0);

		static Ref<ShaderStorageBufferSet> Create(uint32_t count);

	private:
		uint32_t m_count;
		std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, Ref<ShaderStorageBuffer>>>> m_shaderStorageBuffers;
	};
}