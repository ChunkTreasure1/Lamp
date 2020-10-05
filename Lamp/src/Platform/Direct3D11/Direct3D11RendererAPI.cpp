#include "lppch.h"
#include "Direct3D11RendererAPI.h"

#include "Direct3D11Context.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Lamp/Core/Application.h"

#include <d3dcompiler.h>
#include "Platform/Direct3D11/Direct3D11VertexArray.h"

namespace Lamp
{
	void Direct3D11RendererAPI::Initialize()
	{
	}

	void Direct3D11RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				//D3D11_VIEWPORT vp;
				//vp.Width = width;
				//vp.Height = height;
				//vp.MinDepth = 0;
				//vp.MaxDepth = 1;
				//vp.TopLeftX = x;
				//vp.TopLeftY = y;
				//pContext->GetDeviceContext()->RSSetViewports(1u, &vp);
			}
		}
	}

	void Direct3D11RendererAPI::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
	}

	void Direct3D11RendererAPI::Clear()
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				pContext->GetDeviceContext()->ClearRenderTargetView(pContext->GetRenderTarget().Get(), glm::value_ptr(m_ClearColor));
			}
		}
	}

	void Direct3D11RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				namespace wrl = Microsoft::WRL;

				std::vector<float> vertices =
				{
					 0.f,   0.5f,
					 0.5f, -0.5f,
					-0.5f, -0.5f
				};

				wrl::ComPtr<ID3D11VertexShader> pVS;
				wrl::ComPtr<ID3D11PixelShader> pPS;
				wrl::ComPtr<ID3DBlob> pBlob;

				D3DReadFileToBlob(L"PixelShader.cso", &pBlob);
				pContext->GetDevice()->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPS);
				pContext->GetDeviceContext()->PSSetShader(pPS.Get(), nullptr, 0u);

				D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
				pContext->GetDevice()->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVS);
				pContext->GetDeviceContext()->VSSetShader(pVS.Get(), nullptr, 0u);

				Ref<VertexArray> vertexArray = VertexArray::Create();
				Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, (uint32_t)sizeof(float) * vertices.size());
				vertexBuffer->SetBufferLayout
				({
					{ ElementType::Float2, "POSITION" }
				});

				if (auto dVB = std::dynamic_pointer_cast<Direct3D11VertexArray>(vertexArray))
				{
					dVB->SetBlob(pBlob);
				}
				vertexArray->AddVertexBuffer(vertexBuffer);
				vertexArray->Bind();

				pContext->GetDeviceContext()->OMSetRenderTargets(1u, pContext->GetRenderTarget().GetAddressOf(), nullptr);
				pContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				D3D11_VIEWPORT vp;
				vp.Width = 1280;
				vp.Height = 720;
				vp.MinDepth = 0;
				vp.MaxDepth = 1;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;
				pContext->GetDeviceContext()->RSSetViewports(1u, &vp);

				pContext->GetDeviceContext()->Draw(3, 0u);
			}
		}
	}

	void Direct3D11RendererAPI::DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
	}
}