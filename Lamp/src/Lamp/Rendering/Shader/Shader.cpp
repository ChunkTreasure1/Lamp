#include "lppch.h"
#include "Shader.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Direct3D11/Direct3D11Shader.h"

namespace Lamp
{
	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLShader>(path);
			case RendererAPI::API::DX11: return CreateRef<Direct3D11Shader>(path);
		}

		return nullptr;
	}
}