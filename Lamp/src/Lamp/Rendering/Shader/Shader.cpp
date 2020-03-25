#include "lppch.h"
#include "Shader.h"

namespace Lamp
{
	Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
		: m_ID(0), m_VertexPath(vertexPath), m_FragmentPath(fragmentPath)
	{
		std::string vertexCode;
		std::string fragmentCode;

		//Read the vertex shader
		std::ifstream fragmentFile(fragmentPath);
		if (fragmentFile.fail())
		{
			perror(fragmentPath.c_str());
			LP_CORE_ERROR("Failed to open" + fragmentPath + "!");
		}

		std::string line;

		while (std::getline(fragmentFile, line))
		{
			fragmentCode += line + "\n";
		}

		fragmentFile.close();

		//Read the vertex shader
		std::ifstream vertexFile(vertexPath);
		if (vertexFile.fail())
		{
			perror(vertexPath.c_str());
			LP_CORE_ERROR("Failed to open" + vertexPath + "!");
		}

		while (std::getline(vertexFile, line))
		{
			vertexCode += line + "\n";
		}

		vertexFile.close();

		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		m_ID = glCreateProgram();
		glAttachShader(m_ID, vertex);
		glAttachShader(m_ID, fragment);
		glLinkProgram(m_ID);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void Shader::Bind()
	{
		glUseProgram(m_ID);
	}

	void Shader::UploadBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)value);
	}

	void Shader::UploadInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
	}

	void Shader::UploadFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
	}

	void Shader::UploadFloat3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3f(glGetUniformLocation(m_ID, name.c_str()), value.x, value.y, value.z);
	}

	void Shader::UploadFloat4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4f(glGetUniformLocation(m_ID, name.c_str()), value.x, value.y, value.z, value.w);
	}

	void Shader::UploadMat4(const std::string& name, const glm::mat4& mat)
	{
		uint32_t transfromLoc = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix4fv(transfromLoc, 1, GL_FALSE, glm::value_ptr(mat));
	}
	std::shared_ptr<Shader> Shader::Create(const std::string& vertexPath, const std::string& fragmentPath)
	{
		return std::make_shared<Shader>(vertexPath, fragmentPath);
	}
}