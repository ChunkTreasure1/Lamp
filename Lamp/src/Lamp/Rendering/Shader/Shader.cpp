#include "lppch.h"
#include "Shader.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Lamp
{
	Ref<Shader> Shader::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLShader>(path);
		}

		return nullptr;
	}
}