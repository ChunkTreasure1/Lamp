#include "lppch.h"
#include "Texture2D.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture2D.h"
#include "Platform/Vulkan/VulkanTexture2D.h"

namespace Lamp
{
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(width, height);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanTexture2D>(width, height);
		}

		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::filesystem::path& path, bool generateMips)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(path);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanTexture2D>(path, generateMips);
		}

		return nullptr;
	}
}