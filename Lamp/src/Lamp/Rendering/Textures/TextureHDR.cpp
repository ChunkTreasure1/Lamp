#include "lppch.h"
#include "TextureHDR.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLTextureHDR.h"

namespace Lamp
{
	Ref<TextureHDR> TextureHDR::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTextureHDR>(width, height);
		}

		return nullptr;
	}

	Ref<TextureHDR> TextureHDR::Create(const std::filesystem::path& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTextureHDR>(path);
		}

		return nullptr;
	}
}