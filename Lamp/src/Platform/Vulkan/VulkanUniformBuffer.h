#pragma once

#include "Lamp/Rendering/Buffers/UniformBuffer.h"

#include <vulkan/vulkan_core.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class VulkanUniformBuffer : public UniformBuffer
	{
	public:
		VulkanUniformBuffer(const void* data, uint32_t size);
		~VulkanUniformBuffer() override;

		void SetData(const void* data, uint32_t size, uint32_t offset /* = 0 */) override;

		inline VkDescriptorBufferInfo GetDescriptorInfo() const { return m_descriptorInfo; }

	private:
		uint32_t m_size;

		VkDescriptorBufferInfo m_descriptorInfo;
		VkBuffer m_buffer;
		VmaAllocation m_bufferAllocation;
	};
}