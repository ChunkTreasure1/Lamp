#include "lppch.h"
#include "Direct3D11RendererAPI.h"

#include "Direct3D11Context.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Lamp/Core/Application.h"

#include <d3dcompiler.h>
#include "Platform/Direct3D11/Direct3D11VertexArray.h"
#include "Platform/Direct3D11/Direct3D11Shader.h"

#include <DirectXMath.h>

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
				D3D11_VIEWPORT vp;
				vp.Width = width;
				vp.Height = height;
				vp.MinDepth = 0;
				vp.MaxDepth = 1;
				vp.TopLeftX = x;
				vp.TopLeftY = y;
				pContext->GetDeviceContext()->RSSetViewports(1u, &vp);
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

	void Direct3D11RendererAPI::ClearDepth()
	{

	}

	void Direct3D11RendererAPI::OffsetPolygon(float factor, float unit)
	{

	}

	void Direct3D11RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t size, Ref<Shader> shader)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				namespace wrl = Microsoft::WRL;

				vertexArray->Bind();

				pContext->GetDeviceContext()->OMSetRenderTargets(1u, pContext->GetRenderTarget().GetAddressOf(), nullptr);
				pContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				pContext->GetDeviceContext()->DrawIndexed(size, 0u, 0u);
			}
		}
	}

	void Direct3D11RendererAPI::DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
	}
}