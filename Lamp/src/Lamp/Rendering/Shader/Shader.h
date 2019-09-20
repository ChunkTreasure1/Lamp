#pragma once

#include <string>
#include <GL/glew.h>

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
