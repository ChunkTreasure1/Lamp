#include "lppch.h"
#include "Shader.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Direct3D11/Direct3D11Shader.h"

namespace Lamp
{
	Ref<Shader> Shader::Create(std::initializer_list<std::string> paths)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLShader>(paths);
			case RendererAPI::API::DX11: return CreateRef<Direct3D11Shader>(paths);
		}

		return nullptr;
	}
}