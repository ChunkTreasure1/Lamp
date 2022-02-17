#pragma once

#include "Lamp/Core/Core.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	enum class DrawAccess
	{
		Static,
		Dynamic
	};

	class ShaderStorageBuffer
	{
	public:
		ShaderStorageBuffer(uint32_t size);
		~ShaderStorageBuffer();
		void SetData(const void* data, uint32_t size);
		void* Map();
		void Unmap();

		inline VkDescriptorBufferInfo GetDescriptorInfo() { return m_descriptorInfo; }

		static Ref<ShaderStorageBuffer> Create(uint32_t size);

	private:
		VkDescriptorBufferInfo m_descriptorInfo;
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
	};
}