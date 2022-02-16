#include "lppch.h"
#include "VulkanBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include "Lamp/Rendering/CommandBuffer.h"

namespace Lamp
{
	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
	{
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const std::vector<float>& vertices, uint32_t size)
	{ }

	VulkanVertexBuffer::VulkanVertexBuffer(const std::vector<Vertex>& vertices, uint32_t newSize)
	{
		auto device = VulkanContext::GetCurrentDevice();

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;

		VkDeviceSize size = newSize;

		stagingBufferMemory = Utility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		VulkanAllocator allocator;
		void* data = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(data, vertices.data(), size);
		allocator.UnmapMemory(stagingBufferMemory);

		m_bufferAllocation = Utility::CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_buffer);
		Utility::CopyBuffer(stagingBuffer, m_buffer, size);

		allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();

		VulkanAllocator allocator;
		allocator.DestroyBuffer(m_buffer, m_bufferAllocation);
	}

	void VulkanVertexBuffer::Bind() const
	{

	}

	void VulkanVertexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffers[] = { m_buffer };
		vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), 0, 1, buffers, offsets);
	}

	void VulkanVertexBuffer::Unbind() const
	{
	}

	void VulkanVertexBuffer::SetVertices(std::vector<Vertex>& pVertices, uint32_t size)
	{
	}

	void VulkanVertexBuffer::SetData(const void* data, uint32_t newSize)
	{
		auto device = VulkanContext::GetCurrentDevice();

		VulkanAllocator allocator;
		if (m_buffer != VK_NULL_HANDLE)
		{
			allocator.DestroyBuffer(m_buffer, m_bufferAllocation);
		}

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;

		VkDeviceSize size = newSize;
		stagingBufferMemory = Utility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		void* mappedData = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(mappedData, data, size);
		allocator.UnmapMemory(stagingBufferMemory);

		m_bufferAllocation = Utility::CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_buffer);
		Utility::CopyBuffer(stagingBuffer, m_buffer, size);

		allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);
	}

	/////Index buffer/////

	VulkanIndexBuffer::VulkanIndexBuffer(const std::vector<uint32_t>& indices, uint32_t count)
		: m_count(count)
	{
		auto device = VulkanContext::GetCurrentDevice();

		VkDeviceSize size = m_count * sizeof(uint32_t);

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		stagingBufferMemory = Utility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		VulkanAllocator allocator("Index buffer");

		void* data = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(data, indices.data(), size);
		allocator.UnmapMemory(stagingBufferMemory);

		m_bufferAllocation = Utility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_buffer);
		Utility::CopyBuffer(stagingBuffer, m_buffer, size);

		allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
	{
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();

		VulkanAllocator allocator;
		allocator.DestroyBuffer(m_buffer, m_bufferAllocation);
	}

	void VulkanIndexBuffer::Bind() const
	{
	}

	void VulkanIndexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
	{
		vkCmdBindIndexBuffer(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), m_buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanIndexBuffer::Unbind() const
	{
	}

	uint32_t VulkanIndexBuffer::GetCount()
	{
		return m_count;
	}
}