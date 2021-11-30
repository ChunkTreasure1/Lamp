#include "lppch.h"
#include "OpenGLShader.h"

namespace Lamp
{
	namespace Utils
	{
		static std::filesystem::path GetCacheDirectory()
		{
			return "engine/shaders/cache/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			if (!std::filesystem::exists(GetCacheDirectory()))
			{
				std::filesystem::create_directories(GetCacheDirectory());
			}
		}

		static std::string ReadFromFile(const std::filesystem::path& path)
		{
			std::string result;
			std::ifstream in(path, std::ios::in | std::ios::binary);
			if (in)
			{
				in.seekg(0, std::ios::end);
				result.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&result[0], result.size());
			}
			else
			{
				LP_CORE_ASSERT(false, "Unable to read shader!");
			}

			in.close();

			return result;
		}

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
			{
				return GL_VERTEX_SHADER;
			}
			else if (type == "fragment")
			{
				return GL_FRAGMENT_SHADER;
			}
			else if (type == "compute")
			{
				return GL_COMPUTE_SHADER;
			}
		}
	}

	OpenGLShader::OpenGLShader(const std::filesystem::path& path, bool forceCompile)
	{
		Path = path;

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = Utils::ReadFromFile(path);
		auto shaderSources = PreProcess(source);


	}

	OpenGLShader::~OpenGLShader()
	{
	}

	void OpenGLShader::Reload(bool forceCompile)
	{
	}

	void OpenGLShader::Bind()
	{
	}

	const std::string& OpenGLShader::GetName()
	{
		// TODO: insert return statement here
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			LP_CORE_ASSERT(eol != std::string::npos, "Syntax error in shader!");

			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			LP_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel" || type == "compute", "Invalid shader type!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			auto shaderType = Utils::ShaderTypeFromString(type);
			shaderSources[shaderType] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& sources)
	{
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
	}

	void OpenGLShader::CreateProgram()
	{
	}
}