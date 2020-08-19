#include "lppch.h"
#include "OpenGLShader.h"

namespace Lamp
{
	OpenGLShader::OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath)
		: m_RendererID(0), m_FragmentPath(fragmentPath), m_VertexPath(vertexPath)
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

		std::getline(vertexFile, line);
		m_Type = ShaderTypeFromString(line);

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
			LP_ERROR("Vertex shader compilation failed: " + std::string(infoLog) + ". At: " + vertexPath);
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			LP_ERROR("Fragment shader compilation failed: " + std::string(infoLog) + ". At: " + fragmentPath);
		}

		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, vertex);
		glAttachShader(m_RendererID, fragment);
		glLinkProgram(m_RendererID);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::UploadBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), (int)value);
	}

	void OpenGLShader::UploadInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name.c_str()), value);
	}

	void OpenGLShader::UploadFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(m_RendererID, name.c_str()), value);
	}

	void OpenGLShader::UploadFloat3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y, value.z);
	}

	void OpenGLShader::UploadFloat4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadMat4(const std::string& name, const glm::mat4& mat)
	{
		uint32_t transfromLoc = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(transfromLoc, 1, GL_FALSE, glm::value_ptr(mat));
	}
	void OpenGLShader::UploadMat3(const std::string& name, const glm::mat3& mat)
	{
		uint32_t transfromLoc = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(transfromLoc, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void OpenGLShader::UploadIntArray(const std::string& name, int* values, uint32_t count) const
	{
		glUniform1iv(glGetUniformLocation(m_RendererID, name.c_str()), count, values);
	}
}