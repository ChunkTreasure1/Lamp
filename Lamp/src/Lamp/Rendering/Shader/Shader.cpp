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

	ShaderType Shader::ShaderTypeFromString(const std::string& s)
	{
		if (s == "//Illum")
		{
			return ShaderType::Illum;
		}
		else if (s == "//Blinn")
		{
			return ShaderType::Blinn;
		}
		else if (s == "//Phong")
		{
			return ShaderType::Phong;
		}
		else if (s == "BlinnPhong")
		{
			return ShaderType::BlinnPhong;
		}

		return ShaderType::Unknown;
	}
}