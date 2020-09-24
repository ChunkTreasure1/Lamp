#include "lppch.h"
#include "GraphicsContext.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace Lamp
{
	Scope<GraphicsContext> GraphicsContext::Create(void* data)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_CRITICAL("None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(data));
		}

		return nullptr;
	}
}