#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

#include <glad/glad.h>

namespace Lamp
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::filesystem::path& path, bool forceCompile);
		~OpenGLShader() override;

		void Reload(bool forceCompile) override;
		void Bind() override;
		const std::string& GetName() override;

	private:
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& sources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();

	private:
	};
}