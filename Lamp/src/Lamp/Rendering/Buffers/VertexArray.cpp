#include "lppch.h"
#include "VertexArray.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Lamp
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexArray>();
			case RendererAPI::API::Vulkan: return CreateRef<VulkanVertexArray>();
		}

		return nullptr;
	}
}