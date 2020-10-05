#include "lppch.h"
#include "VertexBuffer.h"
#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Direct3D11/Direct3D11Buffer.h"

namespace Lamp
{
	Ref<VertexBuffer> VertexBuffer::Create(std::vector<Vertex>& pVertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(pVertices, size);
			case RendererAPI::API::DX11: return CreateRef<Direct3D11VertexBuffer>(pVertices, size);
		}

		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(size);
			case RendererAPI::API::DX11: return CreateRef<Direct3D11VertexBuffer>(size);

		}

		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(std::vector<float>& vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(vertices, size);
			case RendererAPI::API::DX11: return CreateRef<Direct3D11VertexBuffer>(vertices, size);
		}

		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(std::vector<uint32_t>& pIndices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(pIndices, count);
			case RendererAPI::API::DX11: return CreateRef<Direct3D11IndexBuffer>(pIndices, count);
		}

		return nullptr;
	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* pIndices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(pIndices, count);
			case RendererAPI::API::DX11: return CreateRef<Direct3D11IndexBuffer>(pIndices, count);
		}

		return nullptr;
	}
}