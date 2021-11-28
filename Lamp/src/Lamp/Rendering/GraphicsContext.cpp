#include "lppch.h"
#include "GraphicsContext.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace Lamp
{
	Ref<GraphicsContext> GraphicsContext::Create(void* data)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_CRITICAL("None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLContext>(static_cast<GLFWwindow*>(data));
			case RendererAPI::API::Vulkan: return CreateRef<VulkanContext>(static_cast<GLFWwindow*>(data));
		}

		return nullptr;
	}
}