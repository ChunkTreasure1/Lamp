#include "lppch.h"
#include "Direct3D11VertexArray.h"

#include "Lamp/Core/Application.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Direct3D11/Direct3D11Context.h"

namespace Lamp
{
	static DXGI_FORMAT ElementTypeToDXEnum(ElementType type)
	{
		switch (type)
		{
		case Lamp::ElementType::Bool: return DXGI_FORMAT_R8_SINT;
		case Lamp::ElementType::Int: return DXGI_FORMAT_R32_SINT;
		case Lamp::ElementType::Float: return DXGI_FORMAT_R32_FLOAT;
		case Lamp::ElementType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
		case Lamp::ElementType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case Lamp::ElementType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case Lamp::ElementType::Mat3: return DXGI_FORMAT_R32_FLOAT;
		case Lamp::ElementType::Mat4: return DXGI_FORMAT_R32_FLOAT;
		}
	}

	Direct3D11VertexArray::Direct3D11VertexArray()
		: m_NumAttributes(0), m_IndexBuffer(nullptr), m_pBlob(nullptr), m_pInputLayout(nullptr)
	{
	}

	Direct3D11VertexArray::~Direct3D11VertexArray()
	{
	}

	void Direct3D11VertexArray::Bind()
	{
		if (m_VertexBuffers.size() > 0)
		{
			m_VertexBuffers[0]->Bind();
		}
	}

	void Direct3D11VertexArray::Unbind()
	{
	}

	void Direct3D11VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer)
	{
		pVertexBuffer->Bind();

		std::vector<D3D11_INPUT_ELEMENT_DESC> ied;

		for (auto& element : pVertexBuffer->GetBufferLayout().GetElements())
		{
			ied.push_back({ element.Name.c_str(), 0, ElementTypeToDXEnum(element.ElementType), 0, (uint32_t)element.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			m_NumAttributes++;
		}

		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				//LP_ASSERT(!m_pBlob.Get(), "No vertex shader blob set!");

				pContext->GetDevice()->CreateInputLayout(&ied[0], m_NumAttributes, m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), &m_pInputLayout);
				pContext->GetDeviceContext()->IASetInputLayout(m_pInputLayout.Get());
			}
		}

		m_VertexBuffers.push_back(pVertexBuffer);
	}

	void Direct3D11VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer)
	{
		pIndexBuffer->Bind();
		m_IndexBuffer = pIndexBuffer;
	}
}