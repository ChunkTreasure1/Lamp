#include "lppch.h"
#include "OpenGLShader.h"

namespace Lamp
{
	OpenGLShader::OpenGLShader(std::initializer_list<std::string> paths)
		: m_Paths(paths), m_RendererID(0), m_FragmentPath(m_Paths[1]), m_VertexPath(m_Paths[0])
	{
		std::string vertexCode;
		std::string fragmentCode;

		//Read the vertex shader
		std::ifstream fragmentFile(m_Paths[1]);
		if (fragmentFile.fail())
		{
			perror(m_Paths[1].c_str());
			LP_CORE_ERROR("Failed to open" + m_Paths[1] + "!");
		}

		std::string line;

		while (std::getline(fragmentFile, line))
		{
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

	void OpenGLShader::UploadData(const ShaderData& data)
	{
		for (auto& uniform : data.Data)
		{
			switch (uniform.Type)
			{
			case Lamp::ShaderDataType::Bool:
			{
				glUniform1i(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<int*>(uniform.pData));
				break;
			}

			case Lamp::ShaderDataType::Int:
			{
				glUniform1i(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<int*>(uniform.pData));
				break;
			}

			case Lamp::ShaderDataType::Float:
			{
				glUniform1f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<float*>(uniform.pData));
				break;
			}

			case Lamp::ShaderDataType::Float2:
			{
				glm::vec2* p = static_cast<glm::vec2*>(uniform.pData);

				glUniform2f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y);
				break;
			}

			case Lamp::ShaderDataType::Float3:
			{
				glm::vec3* p = static_cast<glm::vec3*>(uniform.pData);
				glUniform3f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y, p->z);
				break;
			}

			case Lamp::ShaderDataType::Float4:
			{
				glm::vec4* p = static_cast<glm::vec4*>(uniform.pData);
				glUniform4f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y, p->z, p->w);
				break;
			}

			case Lamp::ShaderDataType::Mat3:
			{
				uint32_t transfromLoc = glGetUniformLocation(m_RendererID, uniform.Name.c_str());
				glUniformMatrix3fv(transfromLoc, 1, GL_FALSE, (float*)uniform.pData);
				break;
			}

			case Lamp::ShaderDataType::Mat4:
			{
				uint32_t transfromLoc = glGetUniformLocation(m_RendererID, uniform.Name.c_str());
				glUniformMatrix4fv(transfromLoc, 1, GL_FALSE, (float*)uniform.pData);
				break;
			}
			case Lamp::ShaderDataType::IntArray:

				glUniform1iv(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), sizeof(uniform.pData) / sizeof(int), (int*)uniform.pData);
				break;
			}
		}
	}
}