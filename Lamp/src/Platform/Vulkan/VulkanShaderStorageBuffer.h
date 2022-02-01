#pragma once

#include "Lamp/Rendering/Buffers/ShaderStorageBuffer.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <vulkan/vulkan.h>

namespace Lamp
{
	class VulkanShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		VulkanShaderStorageBuffer(uint32_t size);
		~VulkanShaderStorageBuffer() override;

		void SetData(const void* data, uint32_t size) override;
		void* Map() override;
		void Unmap() override;

		inline const VkDescriptorBufferInfo& GetDescriptorInfo() const { return m_descriptorInfo; }

	private:
		VkDescriptorBufferInfo m_descriptorInfo;
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
	};
}