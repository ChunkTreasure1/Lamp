#include "lppch.h"
#include "ShaderStorageBuffer.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/OpenGL/OpenGLShaderStorageBuffer.h"
#include "Platform/Vulkan/VulkanShaderStorageBuffer.h"

namespace Lamp
{
	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported") return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLShaderStorageBuffer>(size, 0, DrawAccess::Static);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanShaderStorageBuffer>(size);

			default:
				break;
		}

		return nullptr;
	}
}