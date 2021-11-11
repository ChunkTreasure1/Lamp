#pragma once

#include "VertexBuffer.h"

namespace Lamp
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer) = 0;
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;

		virtual Ref<IndexBuffer>& GetIndexBuffer() = 0;

	public:
		static Ref<VertexArray> Create();
	};
}