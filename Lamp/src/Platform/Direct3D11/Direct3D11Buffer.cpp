#include "lppch.h"
#include "Direct3D11Buffer.h"

#include "Lamp/Core/Application.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Direct3D11/Direct3D11Context.h"

namespace Lamp
{
	/////Vertex Buffer/////
	Direct3D11VertexBuffer::Direct3D11VertexBuffer(std::vector<Vertex>& vertices, uint32_t size)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				D3D11_BUFFER_DESC bd = {};
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.CPUAccessFlags = 0u;
				bd.MiscFlags = 0u;
				bd.ByteWidth = size;
				bd.StructureByteStride = sizeof(Vertex);

				D3D11_SUBRESOURCE_DATA sd = {};
				sd.pSysMem = &vertices[0];

				pContext->GetDevice()->CreateBuffer(&bd, &sd, &m_pBuffer);
			}
		}
	}

	Direct3D11VertexBuffer::Direct3D11VertexBuffer(std::vector<float>& vertices, uint32_t size)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				D3D11_BUFFER_DESC bd = {};
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.CPUAccessFlags = 0u;
				bd.MiscFlags = 0u;
				bd.ByteWidth = size;
				bd.StructureByteStride = sizeof(float) * 5;

				D3D11_SUBRESOURCE_DATA sd = {};
				sd.pSysMem = &vertices[0];

				pContext->GetDevice()->CreateBuffer(&bd, &sd, &m_pBuffer);
			}
		}
	}

	Direct3D11VertexBuffer::Direct3D11VertexBuffer(uint32_t size)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				D3D11_BUFFER_DESC bd = {};
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.CPUAccessFlags = 0u;
				bd.MiscFlags = 0u;
				bd.ByteWidth = size;
				bd.StructureByteStride = sizeof(Vertex);

				D3D11_SUBRESOURCE_DATA sd = {};
				sd.pSysMem = nullptr;

				pContext->GetDevice()->CreateBuffer(&bd, &sd, &m_pBuffer);
			}
		}
	}

	Direct3D11VertexBuffer::~Direct3D11VertexBuffer()
	{
	}

	void Direct3D11VertexBuffer::Bind() const
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				const uint32_t stride = sizeof(float) * 5;
				const uint32_t offset = 0u;
				pContext->GetDeviceContext()->IASetVertexBuffers(0u, 1u, m_pBuffer.GetAddressOf(), &stride, &offset);
			}
		}
	}

	void Direct3D11VertexBuffer::Unbind() const
	{
	}

	void Direct3D11VertexBuffer::SetVertices(std::vector<Vertex>& pVertices, uint32_t size)
	{

	}

	void Direct3D11VertexBuffer::SetData(const void* data, uint32_t size)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				D3D11_BUFFER_DESC bd = {};
				bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.CPUAccessFlags = 0u;
				bd.MiscFlags = 0u;
				bd.ByteWidth = size;
				bd.StructureByteStride = sizeof(Vertex);

				D3D11_SUBRESOURCE_DATA sd = {};
				sd.pSysMem = data;

				pContext->GetDevice()->CreateBuffer(&bd, &sd, &m_pBuffer);
			}
		}
	}

	/////Index buffer/////
	Direct3D11IndexBuffer::Direct3D11IndexBuffer(std::vector<uint32_t>& indices, uint32_t count)
		: m_Count(count), m_pBuffer(nullptr)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				D3D11_BUFFER_DESC bd = {};
				bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.CPUAccessFlags = 0u;
				bd.MiscFlags = 0u;
				bd.ByteWidth = count * sizeof(uint32_t);
				bd.StructureByteStride = sizeof(uint32_t);

				D3D11_SUBRESOURCE_DATA sd = {};
				sd.pSysMem = &indices[0];

				pContext->GetDevice()->CreateBuffer(&bd, &sd, &m_pBuffer);
			}
		}
	}

	Direct3D11IndexBuffer::Direct3D11IndexBuffer(uint32_t* pIndices, uint32_t count)
		: m_Count(count), m_pBuffer(nullptr)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				D3D11_BUFFER_DESC bd = {};
				bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bd.Usage = D3D11_USAGE_DEFAULT;
				bd.CPUAccessFlags = 0u;
				bd.MiscFlags = 0u;
				bd.ByteWidth = count * sizeof(uint32_t);
				bd.StructureByteStride = sizeof(uint32_t);

				D3D11_SUBRESOURCE_DATA sd = {};
				sd.pSysMem = pIndices;

				pContext->GetDevice()->CreateBuffer(&bd, &sd, &m_pBuffer);
			}
		}
	}

	Direct3D11IndexBuffer::~Direct3D11IndexBuffer()
	{
	}

	void Direct3D11IndexBuffer::Bind() const
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				pContext->GetDeviceContext()->IASetIndexBuffer(m_pBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
			}
		}
	}

	void Direct3D11IndexBuffer::Unbind() const
	{
	}
}