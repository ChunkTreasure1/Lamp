#pragma once

#include "Lamp/Rendering/Buffers/VertexBuffer.h"

namespace Lamp
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(std::vector<Vertex>& pVertices, uint32_t size);
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(std::vector<float>& vertices, uint32_t size);

		virtual ~OpenGLVertexBuffer() override;

		virtual void Bind() const override;
		virtual void Bind(Ref<CommandBuffer> commandBuffer) const override {}
		virtual void Unbind() const override;

		virtual void SetBufferLayout(const BufferLayout& buff) override { m_Layout = buff; }
		virtual BufferLayout& GetBufferLayout() override { return m_Layout; }

		virtual void SetVertices(std::vector<Vertex>& pVertices, uint32_t size) override;
		virtual void SetData(const void* data, uint32_t size) override;

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(std::vector<uint32_t>& pIndices, uint32_t count);
		OpenGLIndexBuffer(uint32_t* pIndices, uint32_t count);

		virtual ~OpenGLIndexBuffer() override;

		virtual void Bind() const override;
		virtual void Bind(Ref<CommandBuffer> commandBuffer) const override {}
		virtual void Unbind() const override;

		virtual uint32_t GetCount() override { return m_Count; }

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}