#pragma once

#include "Lamp/Core/Core.h"

#include <vulkan/vulkan.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class UniformBuffer
	{
	public:
		UniformBuffer(const void* data, uint32_t size);
		~UniformBuffer();
		void SetData(const void* data, uint32_t size, uint32_t offset = 0);

		inline const VkDescriptorBufferInfo& GetDescriptorInfo() const { return m_descriptorInfo; }

		static Ref<UniformBuffer> Create(const void* data, uint32_t size);

	private:
		uint32_t m_size;

		VkDescriptorBufferInfo m_descriptorInfo;
		VkBuffer m_buffer;
		VmaAllocation m_bufferAllocation;
	};
}