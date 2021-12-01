#include "lppch.h"
#include "UniformBuffer.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Lamp
{
	Ref<UniformBuffer> UniformBuffer::Create(const void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLUniformBuffer>(data, size);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanUniformBuffer>(data, size);
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!") return nullptr;
		default:
			break;
		}

		return nullptr;
	}
}

