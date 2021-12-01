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
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_vulkanSPIRV;
		std::unordered_map<uint32_t, std::vector<uint32_t>> m_openGLSPIRV;

		std::unordered_map<GLenum, std::string> m_openGLSource;

		uint32_t m_rendererId;
		std::string m_name;
	};
}