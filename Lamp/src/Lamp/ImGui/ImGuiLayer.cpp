#include "lppch.h"
#include "ImGuiLayer.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/OpenGL/OpenGLImGuiLayer.h"
#include "Platform/Vulkan/VulkanImGuiLayer.h"

namespace Lamp
{
	ImGuiLayer* ImGuiLayer::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No supported API!") return nullptr;
			case RendererAPI::API::OpenGL: return new OpenGLImGuiLayer();
			case RendererAPI::API::Vulkan: return new VulkanImGuiLayer();
		}

		return nullptr;
	}
}