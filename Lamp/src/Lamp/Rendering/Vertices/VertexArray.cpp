#include "lppch.h"
#include "VertexArray.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Lamp
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return std::make_shared<OpenGLVertexArray>();
		}

		return nullptr;
	}
}