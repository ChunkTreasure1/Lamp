#include "lppch.h"
#include "VulkanShaderStorageBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtility.h"

namespace Lamp
{
	VulkanShaderStorageBuffer::VulkanShaderStorageBuffer(uint32_t size)
	{
		auto device = VulkanContext::GetCurrentDevice();

		VkDeviceSize deviceSize = size;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VulkanAllocator allocator;
		m_allocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_buffer);

		m_descriptorInfo.buffer = m_buffer;
		m_descriptorInfo.offset = 0;
		m_descriptorInfo.range = deviceSize;
	}

	VulkanShaderStorageBuffer::~VulkanShaderStorageBuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();

		VulkanAllocator allocator;
		allocator.DestroyBuffer(m_buffer, m_allocation);
	}

	void VulkanShaderStorageBuffer::SetData(const void* data, uint32_t size)
	{
		VulkanAllocator allocator;
		void* mappedData = allocator.MapMemory<void>(m_allocation);
		memcpy(mappedData, data, size);
		allocator.UnmapMemory(m_allocation);
	}

	void* VulkanShaderStorageBuffer::Map()
	{
		VulkanAllocator allocator;
		return allocator.MapMemory<void>(m_allocation);
	}

	void VulkanShaderStorageBuffer::Unmap()
	{
		VulkanAllocator allocator;
		allocator.UnmapMemory(m_allocation);
	}
}