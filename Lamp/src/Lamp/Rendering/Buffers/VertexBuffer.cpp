#include "lppch.h"
#include "VertexBuffer.h"

#include "Lamp/Rendering/CommandBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtility.h"

namespace Lamp
{
	Ref<VertexBuffer> VertexBuffer::Create(const std::vector<Vertex>& pVertices, uint32_t size)
	{
		return CreateRef<VertexBuffer>(pVertices, size);
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		return CreateRef<VertexBuffer>(size);
	}

	Ref<IndexBuffer> IndexBuffer::Create(const std::vector<uint32_t>& pIndices, uint32_t size)
	{
		return CreateRef<IndexBuffer>(pIndices, size);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* pIndices, uint32_t count)
	{
		return CreateRef<IndexBuffer>(pIndices, count);
	}

	VertexBuffer::VertexBuffer(uint32_t size)
	{}

	VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices, uint32_t newSize)
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

	VertexBuffer::~VertexBuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();

		VulkanAllocator allocator;
		allocator.DestroyBuffer(m_buffer, m_bufferAllocation);
	}

	void VertexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
	{
		VkDeviceSize offsets[] = { 0 };
		VkBuffer buffers[] = { m_buffer };
		vkCmdBindVertexBuffers(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), 0, 1, buffers, offsets);
	}

	void VertexBuffer::SetData(const void* data, uint32_t newSize)
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

	IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices, uint32_t count)
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

	IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
		auto device = VulkanContext::GetCurrentDevice();

		VulkanAllocator allocator;
		allocator.DestroyBuffer(m_buffer, m_bufferAllocation);
	}

	void IndexBuffer::Bind(Ref<CommandBuffer> commandBuffer) const
	{
		vkCmdBindIndexBuffer(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), m_buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	uint32_t IndexBuffer::GetCount()
	{
		return m_count;
	}
}