#include "lppch.h"
#include "GraphicsContext.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Direct3D11/Direct3D11Context.h"

namespace Lamp
{
	Scope<GraphicsContext> GraphicsContext::Create(void* data)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_CRITICAL("None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(data));
			case RendererAPI::API::DX11: return CreateScope<Direct3D11Context>(static_cast<GLFWwindow*>(data));
		}

		return nullptr;
	}
}