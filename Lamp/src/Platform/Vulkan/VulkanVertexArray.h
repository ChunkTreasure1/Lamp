#pragma once

#include "Lamp/Rendering/Buffers/VertexArray.h"

namespace Lamp
{
	class VulkanVertexArray : public VertexArray
	{
	public:
		VulkanVertexArray();
		~VulkanVertexArray() override;

		void Bind() override;
		void Unbind() override;

		void AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer) override;
		void SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer) override;
		const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override;

		Ref<IndexBuffer>& GetIndexBuffer() override;

	private:
		Ref<IndexBuffer> m_indexBuffer;
		std::vector<Ref<VertexBuffer>> m_vertexBuffers;
	};
}