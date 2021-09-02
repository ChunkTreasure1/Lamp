#include "lppch.h"
#include "OpenGLShader.h"

namespace Lamp
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
		{
			return GL_VERTEX_SHADER;
		}
		if (type == "fragment")
		{
			return GL_FRAGMENT_SHADER;
		}
		if (type == "geometry")
		{
			return GL_GEOMETRY_SHADER;
		}

		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& path)
		: m_Path(path), m_RendererID(0)
	{
		std::string source = ReadFile(path);
		auto shaderSources = PreProcess(source);
		
		Compile(shaderSources);
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
			case Lamp::Type::Bool:
			{
				glUniform1i(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<int*>(uniform.Data));
				break;
			}

			case Lamp::Type::Int:
			{
				glUniform1i(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<int*>(uniform.Data));
				break;
			}

			case Lamp::Type::Float:
			{
				glUniform1f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), *static_cast<float*>(uniform.Data));
				break;
			}

			case Lamp::Type::Float2:
			{
				glm::vec2* p = static_cast<glm::vec2*>(uniform.Data);

				glUniform2f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y);
				break;
			}

			case Lamp::Type::Float3:
			{
				glm::vec3* p = static_cast<glm::vec3*>(uniform.Data);
				glUniform3f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y, p->z);
				break;
			}

			case Lamp::Type::Float4:
			{
				glm::vec4* p = static_cast<glm::vec4*>(uniform.Data);
				glUniform4f(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), p->x, p->y, p->z, p->w);
				break;
			}

			case Lamp::Type::Mat3:
			{
				uint32_t transfromLoc = glGetUniformLocation(m_RendererID, uniform.Name.c_str());
				glUniformMatrix3fv(transfromLoc, 1, GL_FALSE, (float*)uniform.Data);
				break;
			}

			case Lamp::Type::Mat4:
			{
				uint32_t transfromLoc = glGetUniformLocation(m_RendererID, uniform.Name.c_str());
				glUniformMatrix4fv(transfromLoc, 1, GL_FALSE, (float*)uniform.Data);
				break;
			}
			case Lamp::Type::IntArray:

				glUniform1iv(glGetUniformLocation(m_RendererID, uniform.Name.c_str()), uniform.Size / sizeof(int), (int*)uniform.Data);
				break;
			}
		}
	}
	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of type declaration line
			LP_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			LP_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); // start of shader code
			LP_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			pos = source.find(typeToken, nextLinePos); //start of next type declaration

			shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		std::string line;
		bool textureNamesStarted = false;

		while (std::getline(in, line))
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

		}

		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				LP_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			LP_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}
	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		std::array<GLenum, 3> glShaderIDs;

		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				LP_CORE_ERROR("{0}", infoLog.data());
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		m_RendererID = program;

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			LP_CORE_ERROR("{0}", infoLog.data());
			LP_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	}
}