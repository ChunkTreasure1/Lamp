#include "lppch.h"
#include "RendererAPI.h"

#include "Lamp/Core/Log.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Lamp
{
#ifdef LP_PLATFORM_WINDOWS
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;
#else
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::DX11;
#endif

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
			case RendererAPI::API::None: LP_CORE_ERROR("None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return std::make_unique<OpenGLRendererAPI>();
		}

		return nullptr;
	}

}