#pragma once

#include "Lamp/Rendering/Vertices/VertexBuffer.h"
#include <wrl.h>
#include <d3d11.h>

namespace Lamp
{
	class Direct3D11VertexBuffer : public VertexBuffer
	{
	public:
		Direct3D11VertexBuffer(std::vector<Vertex>& vertices, uint32_t size);
		Direct3D11VertexBuffer(uint32_t size);
		Direct3D11VertexBuffer(std::vector<float>& vertices, uint32_t size);

		virtual ~Direct3D11VertexBuffer() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetBufferLayout(const BufferLayout& buff) override { m_Layout = buff; }
		virtual BufferLayout& GetBufferLayout() override { return m_Layout; }

		virtual void SetVertices(std::vector<Vertex>& pVertices, uint32_t size) override;
		virtual void SetData(const void* data, uint32_t size) override;

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pBuffer;
		BufferLayout m_Layout;
	};

	class Direct3D11IndexBuffer : public IndexBuffer
	{
	public:
		Direct3D11IndexBuffer(std::vector<uint32_t>& indices, uint32_t count);
		Direct3D11IndexBuffer(uint32_t* pIndices, uint32_t count);

		virtual ~Direct3D11IndexBuffer() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() override { return m_Count; }

	private:
		uint32_t m_Count;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pBuffer;
	};
}