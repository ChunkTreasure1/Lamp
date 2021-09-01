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

	void Direct3D11RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t size)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				namespace wrl = Microsoft::WRL;

				std::vector<float> vertices =
				{
					 -1.0f, -1.0f, -1.0f,  1.f, 0.f, 0.f,
					  1.0f, -1.0f, -1.0f,  0.f, 1.f, 0.f,
					 -1.0f,  1.0f, -1.0f,  0.f, 0.f, 1.f,
					  1.0f,  1.0f, -1.0f,  1.f, 0.f, 0.f,
					 -1.0f, -1.0f,  1.0f,  1.f, 0.f, 0.f,
					  1.0f, -1.0f,  1.0f,  0.f, 1.f, 0.f,
					 -1.0f,  1.0f,  1.0f,  0.f, 0.f, 1.f,
					  1.0f,  1.0f,  1.0f,  1.f, 0.f, 0.f
				};

				Ref<Shader> pShader = Shader::Create({ {"VertexShader.cso"}, {"PixelShader.cso"} });

				Ref<VertexArray> vertexArray = VertexArray::Create();
				Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, (uint32_t)sizeof(float) * vertices.size());
				vertexBuffer->SetBufferLayout
				({
					{ ElementType::Float3, "POSITION" },
					{ ElementType::Float3, "COLOR" }
					});

				std::vector<uint32_t> indices =
				{
					0, 2, 1,  2, 3, 1,
					1, 3, 5,  3, 7, 5,
					2, 6, 3,  3, 6, 7,
					4, 5, 7,  4, 7, 6,
					0, 4, 2,  2, 4, 6,
					0, 1, 4,  1, 5, 4
				};
				Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, indices.size());
				vertexArray->SetIndexBuffer(indexBuffer);

				if (auto dVB = std::dynamic_pointer_cast<Direct3D11VertexArray>(vertexArray))
				{
					if (auto shader = std::dynamic_pointer_cast<Direct3D11Shader>(pShader))
					{
						dVB->SetBlob(shader->GetVertexBlob());
					}
				}

				namespace dx = DirectX;

				auto v = 
					dx::XMMatrixTranslation(0.f, 0.f, 4.f) * dx::XMMatrixPerspectiveFovLH(glm::radians(45.f), 16.f / 9.f, 0.1f, 100.f);

				auto d = glm::perspective(glm::radians(45.f), 16.f / 9.f, 0.1f, 100.f) * glm::translate(glm::mat4(1.f), { 0.f, 0.f, 4.f });

				pShader->UploadData(ShaderData
				({
					{ "u_Model", ShaderDataType::Mat4, glm::value_ptr(d) },
				}));

				vertexArray->AddVertexBuffer(vertexBuffer);
				vertexArray->Bind();

				pContext->GetDeviceContext()->OMSetRenderTargets(1u, pContext->GetRenderTarget().GetAddressOf(), nullptr);
				pContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				pContext->GetDeviceContext()->DrawIndexed((uint32_t)std::size(indices), 0u, 0u);
			}
		}
	}

	void Direct3D11RendererAPI::DrawIndexedLines(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
	}
}