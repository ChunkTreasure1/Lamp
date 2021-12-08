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
		const ShaderSpecification& GetSpecification() const override { return m_specification; }

	private:
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& sources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

		std::unordered_map<uint32_t, std::vector<uint32_t>> m_vulkanSPIRV;
		std::unordered_map<uint32_t, std::vector<uint32_t>> m_openGLSPIRV;

		std::unordered_map<GLenum, std::string> m_openGLSource;

		ShaderSpecification m_specification;
		uint32_t m_rendererId;
	};
}