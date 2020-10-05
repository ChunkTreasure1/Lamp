#pragma once

#include "Lamp/Rendering/Vertices/VertexArray.h"

namespace Lamp
{
	class Direct3D11VertexArray : public VertexArray
	{
	public:
		Direct3D11VertexArray();
		virtual ~Direct3D11VertexArray() override;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer) override;
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		
		virtual Ref<IndexBuffer>& GetIndexBuffer() override { return m_IndexBuffer; }
	
	private:
		uint32_t m_NumAttributes;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}