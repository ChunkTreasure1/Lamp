#include "GLSLProgram.h"
#include "../../Main/Errors.h"

#include <fstream>
#include <vector>
#include <filesystem>

namespace CactusEngine
{
	GLSLProgram::GLSLProgram()
		: m_ProgramID(0), m_VertShaderID(0), m_FragShaderID(0), m_NumAttributes(0)
	{}


	GLSLProgram::~GLSLProgram()
	{}

	//Compile the shaders
	void GLSLProgram::CompileShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		m_ProgramID = glCreateProgram();

		//Create the shaders
		m_VertShaderID = glCreateShader(GL_VERTEX_SHADER);
		if (m_VertShaderID == 0)
		{
			FatalError("Vertex shader failed to be created!");
		}

		m_FragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		if (m_FragShaderID == 0)
		{
			FatalError("Fragment shader failed to be created!");
		}

		CompileShader(vertexShaderPath, m_VertShaderID);
		CompileShader(fragmentShaderPath, m_FragShaderID);
	}

	//Link the shaders
	void GLSLProgram::LinkShaders()
	{
		// Attach our shaders to our program
		glAttachShader(m_ProgramID, m_VertShaderID);
		glAttachShader(m_ProgramID, m_FragShaderID);

		// Link our program
		glLinkProgram(m_ProgramID);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(m_ProgramID, GL_LINK_STATUS, (int*)& isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_ProgramID, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<char> infoLog(maxLength);
			glGetProgramInfoLog(m_ProgramID, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(m_ProgramID);
			// Don't leak shaders either.
			glDeleteShader(m_VertShaderID);
			glDeleteShader(m_FragShaderID);

			// Use the infoLog as you see fit.

			std::printf("%s\n", &(infoLog[0]));
			FatalError("Shaders failed to link!");
		}

		// Always detach shaders after a successful link.
		glDetachShader(m_ProgramID, m_VertShaderID);
		glDetachShader(m_ProgramID, m_FragShaderID);

		glDeleteShader(m_VertShaderID);
		glDeleteShader(m_FragShaderID);
	}

	//Adds an attribute from the shader
	void GLSLProgram::AddAttribute(const std::string& attributeName)
	{
		glBindAttribLocation(m_ProgramID, m_NumAttributes++, attributeName.c_str());
	}

	GLint GLSLProgram::GetUniformLocation(const std::string& uniformName)
	{
		GLint location = glGetUniformLocation(m_ProgramID, uniformName.c_str());
		if (location == GL_INVALID_INDEX)
		{
			FatalError("Uniform" + uniformName + " not found in shader!");
		}
		return location;
	}

	//Set to use this GLSL program
	void GLSLProgram::Use()
	{
		glUseProgram(m_ProgramID);

		for (int i = 0; i < m_NumAttributes; i++)
		{
			glEnableVertexAttribArray(i);
		}
	}

	//Remove the usage of this GLSL program
	void GLSLProgram::Unuse()
	{
		glUseProgram(0);

		for (int i = 0; i < m_NumAttributes; i++)
		{
			glDisableVertexAttribArray(i);
		}
	}

	//Compile a single shader
	void GLSLProgram::CompileShader(const std::string & filePath, GLuint & id)
	{
		//Read the vertex shader
		std::ifstream vertexFile(filePath);
		if (vertexFile.fail())
		{
			perror(filePath.c_str());
			FatalError("Failed to open " + filePath + "!");
		}

		std::string fileContents = "";
		std::string line;

		while (std::getline(vertexFile, line))
		{
			fileContents += line + "\n";
		}

		vertexFile.close();

		//Set the source and compile the shader
		const char* contentsPtr = fileContents.c_str();
		glShaderSource(id, 1, &contentsPtr, nullptr);

		glCompileShader(id);

		//Error checking
		GLint success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<char> errorLog(maxLength);
			glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

			glDeleteShader(id);

			std::printf("%s\n", &(errorLog[0]));
			FatalError("Shader " + filePath + " failed to compile!");
		}
	}
}
