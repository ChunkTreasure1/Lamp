#include "lppch.h"
#include "OpenGLShader.h"

namespace Lamp
{
	OpenGLShader::OpenGLShader(std::initializer_list<std::string> paths)
		: m_Paths(paths), m_RendererID(0), m_FragmentPath(m_Paths[1]), m_VertexPath(m_Paths[0])
	{
		std::string vertexCode;
		std::string fragmentCode;
		std::string geoCode;

		//Read the vertex shader
		std::ifstream fragmentFile(m_Paths[1]);
		if (fragmentFile.fail())
		{
			perror(m_Paths[1].c_str());
			LP_CORE_ERROR("Failed to open" + m_Paths[1] + "!");
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
		std::ifstream vertexFile(m_Paths[0]);
		if (vertexFile.fail())
		{
			perror(m_Paths[0].c_str());
			LP_CORE_ERROR("Failed to open" + m_Paths[0] + "!");
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
			LP_ERROR("Vertex shader compilation failed: " + std::string(infoLog) + ". At: " + m_Paths[0]);
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			LP_ERROR("Fragment shader compilation failed: " + std::string(infoLog) + ". At: " + m_Paths[1]);
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

	void OpenGLShader::UploadData(const ShaderData& data)
	{
		for (auto& uniform : data.Data)
		{
			switch (uniform.Type)
			{
			case Lamp::ShaderDataType::Bool:
			{
				glUniform1i(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<int*>(uniform.Data));
				break;
			}

			case Lamp::ShaderDataType::Int:
			{
				glUniform1i(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<int*>(uniform.Data));
				break;
			}

			case Lamp::ShaderDataType::Float:
			{
				glUniform1f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<float*>(uniform.Data));
				break;
			}

			case Lamp::ShaderDataType::Float2:
			{
				glm::vec2* p = static_cast<glm::vec2*>(uniform.Data);

				glUniform2f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y);
				break;
			}

			case Lamp::ShaderDataType::Float3:
			{
				glm::vec3* p = static_cast<glm::vec3*>(uniform.Data);
				glUniform3f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y, p->z);
				break;
			}

			case Lamp::ShaderDataType::Float4:
			{
				glm::vec4* p = static_cast<glm::vec4*>(uniform.Data);
				glUniform4f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y, p->z, p->w);
				break;
			}

			case Lamp::ShaderDataType::Mat3:
			{
				uint32_t transfromLoc = glGetUniformLocation(m_RendererID, uniform.Name.c_str());
				glUniformMatrix3fv(transfromLoc, 1, GL_FALSE, (float*)uniform.Data);
				break;
			}

			case Lamp::ShaderDataType::Mat4:
			{
				uint32_t transfromLoc = glGetUniformLocation(m_RendererID, uniform.Name.c_str());
				glUniformMatrix4fv(transfromLoc, 1, GL_FALSE, (float*)uniform.Data);
				break;
			}
			case Lamp::ShaderDataType::IntArray:

				glUniform1iv(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), uniform.Size / sizeof(int), (int*)uniform.Data);
				break;
			}
		}
	}
}