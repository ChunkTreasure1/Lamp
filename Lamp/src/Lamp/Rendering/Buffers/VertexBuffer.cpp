#include "lppch.h"
#include "VertexBuffer.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Lamp
{
	Ref<VertexBuffer> VertexBuffer::Create(std::vector<Vertex>& pVertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(pVertices, size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexBuffer>(pVertices, size);
		}

		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexBuffer>(size);
		}

		return nullptr;
	}
	Ref<VertexBuffer> VertexBuffer::Create(std::vector<float>& vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(vertices, size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexBuffer>(vertices, size);
		}

		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(std::vector<uint32_t>& pIndices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(pIndices, size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanIndexBuffer>(pIndices, size);
		}

		return nullptr;
	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* pIndices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(pIndices, count);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanIndexBuffer>(pIndices, count);
		}

		return nullptr;
	}
}