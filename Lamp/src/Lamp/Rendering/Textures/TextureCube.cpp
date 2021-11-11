#include "lppch.h"
#include "TextureCube.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLTextureCube.h"

namespace Lamp
{
	Ref<TextureCube> TextureCube::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTextureCube>(width, height);
		}

		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const std::filesystem::path& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLTextureCube>(path);
		}

		return nullptr;
	}
}