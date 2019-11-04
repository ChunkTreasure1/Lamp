#pragma once

#include <functional>

namespace Lamp
{
	class VertexBuffer
	{
	public:
		VertexBuffer(float* pVertices, uint32_t size);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

	public:
		static VertexBuffer* Create(float* pVertices, uint32_t size);

	private:
		uint32_t m_RendererID;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(uint32_t* pIndices, uint32_t count);
		~IndexBuffer();

		void Bind() const;
		void Unbind() const;

		uint32_t GetCount() { return m_Count; }

	public:
		static IndexBuffer* Create(uint32_t* pIndices, uint32_t size);

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}