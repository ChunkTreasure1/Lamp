#include "lppch.h"
#include "ShaderStorageBuffer.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLShaderStorageBuffer.h"

namespace Lamp
{
	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size, uint32_t binding, DrawAccess access)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLShaderStorageBuffer>(size, binding, access);
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported") return nullptr;

			default:
				break;
		}

		return nullptr;
	}
}