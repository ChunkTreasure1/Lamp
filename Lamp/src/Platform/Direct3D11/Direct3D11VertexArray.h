#pragma once

#include "Lamp/Rendering/Vertices/VertexArray.h"
#include <wrl.h>
#include <d3d11.h>

namespace Lamp
{
	class Direct3D11VertexArray : public VertexArray
	{
	public:
		Direct3D11VertexArray();
		virtual ~Direct3D11VertexArray() override;

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer, Ref<Shader> shader) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer) override;
		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		
		virtual Ref<IndexBuffer>& GetIndexBuffer() override { return m_IndexBuffer; }

		inline void SetBlob(Microsoft::WRL::ComPtr<ID3DBlob>& blob) { m_pBlob = blob; }
	
	private:
		uint32_t m_NumAttributes;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pInputLayout;
		Microsoft::WRL::ComPtr<ID3DBlob> m_pBlob;
	};
}