#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace Lamp
{
	class Shader
	{
	public:
		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		~Shader();

		//Functions
		void LinkShaders();
		void AddAttribute(const std::string& attributeName);
		GLint GetUniformLocation(const std::string& uniformName);

		void Bind();
		void Unbind();

		//Uploads
		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);

		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);

		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:

		//Member functions
		void CompileShader(const std::string& filePath, GLuint& id);

		//Member vars
		GLuint m_ProgramID;
		GLuint m_VertShaderID;
		GLuint m_FragShaderID;

		int m_NumAttributes;
	};

}
