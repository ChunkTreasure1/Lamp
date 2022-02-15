#include "lppch.h"
#include "VulkanVertexArray.h"

namespace Lamp
{
	VulkanVertexArray::VulkanVertexArray()
	{
	}

	VulkanVertexArray::~VulkanVertexArray()
	{
	}

	void VulkanVertexArray::Bind()
	{
	}

	void VulkanVertexArray::Unbind()
	{
	}

	void VulkanVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer)
	{
		m_vertexBuffers.push_back(pVertexBuffer);
	}

	void VulkanVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer)
	{
		m_indexBuffer = pIndexBuffer;
	}

	const std::vector<Ref<VertexBuffer>>& VulkanVertexArray::GetVertexBuffers() const
	{
		return m_vertexBuffers;
	}

	Ref<IndexBuffer>& VulkanVertexArray::GetIndexBuffer()
	{
		return m_indexBuffer;
	}
}