#include "lppch.h"
#include "OpenGLShader.h"

namespace Lamp
{
	namespace Utils
	{
		static UniformType GLUniformToUniformType(GLenum type)
		{
			switch (type)
			{
			case GL_INT: return UniformType::Int;
			case GL_FLOAT: return UniformType::Float;
			case GL_FLOAT_VEC2: return UniformType::Float2;
			case GL_FLOAT_VEC3: return UniformType::Float3;
			case GL_FLOAT_VEC4: return UniformType::Float4;
			case GL_FLOAT_MAT3: return UniformType::Mat3;
			case GL_FLOAT_MAT4: return UniformType::Mat4;
			case GL_SAMPLER_2D: return UniformType::Sampler2D;
			case GL_SAMPLER_CUBE: return UniformType::SamplerCube;

			default:
				return UniformType::Int;
				break;
			}
		}

		static uint32_t GLUniformToSize(GLenum type)
		{
			switch (type)
			{
				case GL_INT: return 4;
				case GL_FLOAT: return 4;
				case GL_FLOAT_VEC2: return 4 * 2;
				case GL_FLOAT_VEC3: return 4 * 3;
				case GL_FLOAT_VEC4: return 4 * 4;
				case GL_FLOAT_MAT3: return 4 * 3 * 3;
				case GL_FLOAT_MAT4: return 4 * 4 * 4;
				case GL_SAMPLER_2D: return 4;
				case GL_SAMPLER_CUBE: return 4;

				default:
					return 1;
					break;
			}
		}

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
	}


	OpenGLShader::OpenGLShader(const std::string& path)
		: m_RendererID(0), m_Path(path)
	{
		std::string source = ReadFile(path);
		auto shaderSources = PreProcess(source);

		Compile(shaderSources);

		int count;
		glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &count);
		for (int i = 0; i < count; i++)
		{
			GLchar name[512];
			int length;
			int size;
			GLenum type;

			std::string n;

			glGetActiveUniform(m_RendererID, (GLuint)i, 512, &length, &size, &type, name);
			if (n.find("["))
			{

			}

			m_Specifications.Uniforms.emplace(std::make_pair(name, Utils::GLUniformToUniformType(type))); //TODO: add support for arrays
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

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		//Get specs
		const char* nameToken = "Name:";
		const char* textureCountToken = "TextureCount:";
		const char* textureNamesToken = "TextureNames";
		size_t nameTokenLength = strlen(nameToken);
		size_t texCountTokenLength = strlen(textureCountToken);
		size_t texNamesTokenLength = strlen(textureNamesToken);
		size_t tokenPos = source.find(nameToken, 0);

		{
			size_t eol = source.find_first_of("\r\n", tokenPos);

			size_t begin = tokenPos + nameTokenLength + 1;
			std::string name = source.substr(begin, eol - begin);
			m_Specifications.Name = name;
			tokenPos = std::string::npos;
		}

		{
			tokenPos = source.find(textureCountToken, 0);

			size_t eol = source.find_first_of("\r\n", tokenPos);

			size_t begin = tokenPos + texCountTokenLength + 1;
			std::string name = source.substr(begin, eol - begin);
			m_Specifications.TextureCount = stoi(name);
		}

		{
			tokenPos = source.find(textureNamesToken, 0);

			size_t eol = source.find_first_of("\r\n", tokenPos);
			eol = source.find_first_of("\r\n", eol + 2);

			for (int i = 0; i < m_Specifications.TextureCount; i++)
			{
				size_t pos = source.find_first_of("\r\n", eol + 2);
				std::string name = source.substr(eol + 2, pos - eol - 2);
				m_Specifications.TextureNames.push_back(name);

				eol = pos;
			}
		}

		std::unordered_map<GLenum, std::string> shaderSources;

		//Divide up code
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of type declaration line
			LP_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			LP_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); // start of shader code
			LP_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			pos = source.find(typeToken, nextLinePos); //start of next type declaration

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
			switch (Utils::ShaderTypeFromString(type))
			{
			case GL_VERTEX_SHADER:
				m_Specifications.Type |= ShaderType::VertexShader;
				break;

			case GL_FRAGMENT_SHADER:
				m_Specifications.Type |= ShaderType::FragmentShader;
				break;

			case GL_GEOMETRY_SHADER:
				m_Specifications.Type |= ShaderType::GeometryShader;
				break;

			default:
				m_Specifications.Type = ShaderType::VertexShader;
				break;
			}
		}

		for (auto& source : shaderSources)
		{
			std::string& s = source.second;
			const char* includeToken = "#include";
			size_t includeTokenLength = strlen(includeToken);
			size_t includePos = s.find(includeToken, 0);
			while (includePos != std::string::npos)
			{
				size_t eol = s.find_first_of("\r\n", includePos);

				size_t begin = includePos + includeTokenLength + 1;
				//Get include path and remove from string
				std::string includePath = s.substr(begin, eol - begin);
				s.erase(includePos, eol - includePos);

				std::string startPath = m_Path.substr(0, m_Path.find_last_of("/\\") + 1);
				std::string includeSource = ReadFile(startPath + includePath);

				s.insert(includePos, includeSource);

				size_t nextLinePos = s.find_first_not_of("\r\n", eol);
				includePos = s.find(includeToken, nextLinePos);
			}
		}

		return shaderSources;
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

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
				LP_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			LP_ERROR("Could not open file '{0}'", filepath);
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

				LP_ERROR("{0}", infoLog.data());
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
			GLint maxLength = 512;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(512);
			glGetProgramInfoLog(program, maxLength, NULL, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			LP_ERROR("{0}", infoLog.data());
			LP_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	}

	void OpenGLShader::Recompile()
	{
		std::string source = ReadFile(m_Path);
		auto shaderSources = PreProcess(source);

		Compile(shaderSources);
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