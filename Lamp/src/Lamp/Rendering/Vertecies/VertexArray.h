#pragma once

#include "VertexBuffer.h"

namespace Lamp
{
	class VertexArray
	{
	public:
		VertexArray();
		~VertexArray();

		void Bind();
		void Unbind();

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& pVertexBuffer);
		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& pIndexBuffer);

		std::shared_ptr<IndexBuffer>& GetIndexBuffer() { return m_pIndexBuffer; }

	public:
		static std::shared_ptr<VertexArray> Create();

	private:
		uint32_t m_RendererID;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_pIndexBuffer;
	};
}