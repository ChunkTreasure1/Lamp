#include "lppch.h"
#include "OpenGLShader.h"

namespace Lamp
{
	OpenGLShader::OpenGLShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geoPath)
		: m_RendererID(0), m_FragmentPath(fragmentPath), m_VertexPath(vertexPath)
	{
		std::string vertexCode;
		std::string fragmentCode;
		std::string geoCode;

		//Read the fragment shader
		std::ifstream fragmentFile(fragmentPath);
		if (fragmentFile.fail())
		{
			perror(fragmentPath.c_str());
			LP_CORE_ERROR("Failed to open" + fragmentPath + "!");
		}

		std::string line;
		bool textureNamesStarted = false;

		while (std::getline(fragmentFile, line))
		{
			if (line.find("#ShaderSpec") != std::string::npos)
			{
				continue;
			}

			if (line.find("Name") != std::string::npos && line.find("Texture") == std::string::npos)
			{
				std::string s = line.substr(line.find_first_of(":") + 1, line.size() - line.find_first_of(":"));
				s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
				s.erase(std::remove(s.begin(), s.end(), ';'), s.end());

				m_Specifications.Name = s;
				continue;
			}

			if (line.find("TextureCount") != std::string::npos)
			{
				//If this fails, check for a typo in the shader spec
				std::string s = line.substr(line.find_first_of(':') + 1, line.size() - line.find_first_of(':'));
				s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
				s.erase(std::remove(s.begin(), s.end(), 
					';'), s.end());
				
				m_Specifications.TextureCount = std::atoi(s.c_str());
				continue;
			}

			if (line.find("TextureNames") != std::string::npos)
			{
				textureNamesStarted = true;
				continue;
			}

			if (textureNamesStarted && line.find("{") == std::string::npos)
			{
				if (line.find("}") != std::string::npos)
				{
					textureNamesStarted = false;
					continue;
				}

				m_Specifications.TextureNames.push_back(line);
				continue;
			}

			if (line.find("{") != std::string::npos && textureNamesStarted)
			{
				continue;
			}

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

		//Read the geometry shader
		if (geoPath != "")
		{
			std::ifstream geoFile(geoPath);
			if (geoFile.fail())
			{
				perror(geoPath.c_str());
				LP_CORE_ERROR("Failed to open" + geoPath + "!");
			}

			while (std::getline(geoFile, line))
			{
				geoCode += line + "\n";
			}

			geoFile.close();
		}

		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		unsigned int vertex, fragment, geometry;
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

		if (geoPath != "")
		{
			const char* gGeoCode = geoCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gGeoCode, NULL);
			glCompileShader(geometry);

			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				LP_ERROR("Geometry shader compilation failed: " + std::string(infoLog) + ". At: " + fragmentPath);
			}
		}

		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, vertex);
		glAttachShader(m_RendererID, fragment);

		if (geoPath != "")
		{
			glAttachShader(m_RendererID, geometry);
		}

		glLinkProgram(m_RendererID);
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(m_RendererID, 512, NULL, infoLog);
			LP_ERROR("Shader link failed: " + std::string(infoLog) + ". At: " + m_Specifications.Name);
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		if (geoPath != "")
		{
			glDeleteShader(geometry);
		}
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

	void OpenGLShader::UploadFloat2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2f(glGetUniformLocation(m_RendererID, name.c_str()), value.x, value.y);
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