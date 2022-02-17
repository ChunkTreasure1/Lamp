#include "lppch.h"
#include "UniformBuffer.h"

#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanAllocator.h"

namespace Lamp
{
	UniformBuffer::UniformBuffer(const void* data, uint32_t size)
		: m_size(size)
	{
		uint32_t framesInFlight = Renderer::Get().GetCapabilities().framesInFlight;
		auto device = VulkanContext::GetCurrentDevice();

		VkDeviceSize deviceSize = size;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = deviceSize;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VulkanAllocator allocator;
		m_bufferAllocation = allocator.AllocateBuffer(bufferInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_buffer);

		void* mappedData = allocator.MapMemory<void>(m_bufferAllocation);
		memcpy(mappedData, data, size);
		allocator.UnmapMemory(m_bufferAllocation);

		m_descriptorInfo.buffer = m_buffer;
		m_descriptorInfo.offset = 0;
		m_descriptorInfo.range = deviceSize;
	}

	UniformBuffer::~UniformBuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();

		VulkanAllocator allocator;
		allocator.DestroyBuffer(m_buffer, m_bufferAllocation);
	}

	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		VulkanAllocator allocator;
		void* mappedData = allocator.MapMemory<void>(m_bufferAllocation);
		memcpy(mappedData, data, m_size);
		allocator.UnmapMemory(m_bufferAllocation);
	}

	Ref<UniformBuffer> UniformBuffer::Create(const void* data, uint32_t size)
	{
		return CreateRef<UniformBuffer>(data, size);
	}
}

