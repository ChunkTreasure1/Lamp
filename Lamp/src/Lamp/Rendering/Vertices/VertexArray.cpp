#include "lppch.h"
#include "VertexArray.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/Direct3D11/Direct3D11VertexArray.h"

namespace Lamp
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexArray>();
			case RendererAPI::API::DX11: return CreateRef<Direct3D11VertexArray>();
		}

		return nullptr;
	}
}