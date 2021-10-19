#include "lppch.h"
#include "UniformBuffer.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace Lamp
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLUniformBuffer>(size, binding);
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!") return nullptr;
		default:
			break;
		}

		return nullptr;
	}
}

