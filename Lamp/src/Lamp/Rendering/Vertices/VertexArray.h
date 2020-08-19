#pragma once

#include "VertexBuffer.h"

namespace Lamp
{
	class VertexArray
	{
	public:
		//VertexArray();
		virtual ~VertexArray() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer) = 0;
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;

		virtual Ref<IndexBuffer>& GetIndexBuffer() = 0;

	public:
		static Ref<VertexArray> Create();

	//private:
	//	uint32_t m_RendererID;
	//	uint32_t m_NumAttributes = 0;
	//	std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
	//	std::shared_ptr<IndexBuffer> m_pIndexBuffer;
	};
}