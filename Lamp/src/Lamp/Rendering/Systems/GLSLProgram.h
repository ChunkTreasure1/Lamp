#pragma once

#include <string>
#include <GL/glew.h>

namespace Lamp
{
	class GLSLProgram
	{
	public:
		GLSLProgram();
		~GLSLProgram();

		//Functions
		void CompileShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		void LinkShaders();
		void AddAttribute(const std::string& attributeName);
		GLint GetUniformLocation(const std::string& uniformName);

		void Use();
		void Unuse();

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
