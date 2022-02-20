#include "lppch.h"
#include "VertexBuffer.h"

#include "Platform/Vulkan/VulkanBuffer.h"

namespace Lamp
{
	Ref<VertexBuffer> VertexBuffer::Create(const std::vector<Vertex>& pVertices, uint32_t size)
	{
		return CreateRef<VulkanVertexBuffer>(pVertices, size);
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		return CreateRef<VulkanVertexBuffer>(size);
	}

	Ref<VertexBuffer> VertexBuffer::Create(const std::vector<float>& vertices, uint32_t size)
	{
		return CreateRef<VulkanVertexBuffer>(vertices, size);
	}

	Ref<IndexBuffer> IndexBuffer::Create(const std::vector<uint32_t>& pIndices, uint32_t size)
	{
		return CreateRef<VulkanIndexBuffer>(pIndices, size);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* pIndices, uint32_t count)
	{
		return CreateRef<VulkanIndexBuffer>(pIndices, count);
	}
}