#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& path);
		virtual ~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;
		void Recompile() override;

		void UploadBool(const std::string& name, bool value) const override;
		void UploadInt(const std::string& name, int value) const override;
		void UploadFloat(const std::string& name, float value) const override;
		void UploadFloat2(const std::string& name, const glm::vec2& value) const override;
		void UploadFloat3(const std::string& name, const glm::vec3& value) const override;
		void UploadFloat4(const std::string& name, const glm::vec4& value) const override;

		void UploadMat4(const std::string& name, const glm::mat4& mat) override;
		void UploadMat3(const std::string& name, const glm::mat3& mat) override;
		void UploadIntArray(const std::string& name, int* values, uint32_t count) const override;

		const std::string& GetName() override { return m_specification.name; }
		std::string& GetPath() override { return m_Path; }

	private:
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		std::string ReadFile(const std::string& filepath);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);


	private:
		uint32_t m_RendererID;
		std::string m_Path;
	};
}