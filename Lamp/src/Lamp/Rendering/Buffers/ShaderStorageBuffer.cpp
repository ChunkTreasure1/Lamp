#include "lppch.h"
#include "ShaderStorageBuffer.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLShaderStorageBuffer.h"

namespace Lamp
{
	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLShaderStorageBuffer>(size, binding);
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported") return nullptr;

			default:
				break;
		}

		return nullptr;
	}
}