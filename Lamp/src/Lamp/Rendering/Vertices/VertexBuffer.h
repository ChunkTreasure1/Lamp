#pragma once

#include <functional>
#include "BufferLayout.h"

namespace Lamp
{
	class VertexBuffer
	{
	public:
		VertexBuffer(float* pVertices, uint32_t size);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

		inline void SetBufferLayout(const BufferLayout& buff) { m_BufferLayout = buff; }
		inline BufferLayout& GetBufferLayout() { return m_BufferLayout; }

	public:
		static VertexBuffer* Create(float* pVertices, uint32_t size);

	private:
		uint32_t m_RendererID;
		BufferLayout m_BufferLayout;
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