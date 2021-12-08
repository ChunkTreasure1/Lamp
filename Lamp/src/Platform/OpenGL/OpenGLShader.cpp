#include "lppch.h"
#include "OpenGLShader.h"

#include <shaderc/shaderc.hpp>

#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

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

		static std::string GLShaderStageCahcedVulkanFileExtension(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return ".cached_vulkan.vert";
				case GL_FRAGMENT_SHADER: return ".cached_vulkan.frag";
				case GL_COMPUTE_SHADER: return ".cached_vulkan_compute";
			}

			return "";
		}

		static std::string GLShaderStageCahcedOpenGLFileExtension(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return ".cached_opengl.vert";
				case GL_FRAGMENT_SHADER: return ".cached_opengl.frag";
				case GL_COMPUTE_SHADER: return ".cached_opengl_compute";
			}

			return "";
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
				case GL_COMPUTE_SHADER: return shaderc_glsl_compute_shader;
			}

			return (shaderc_shader_kind)0;
		}

		static std::string GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return "VERTEX_SHADER";
				case GL_FRAGMENT_SHADER: return "FRAGMENT_SHADER";
				case GL_COMPUTE_SHADER: return "COMPUTE_SHADER";
			}

			return "NONE";
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
		return m_specification.name;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		//Get specs
		const char* nameToken = "Name:";
		const char* textureCountToken = "TextureCount:";
		const char* textureNamesToken = "TextureNames";
		const char* internalShaderToken = "InternalShader:";
		size_t nameTokenLength = strlen(nameToken);
		size_t texCountTokenLength = strlen(textureCountToken);
		size_t texNamesTokenLength = strlen(textureNamesToken);
		size_t internalShaderTokenLength = strlen(internalShaderToken);

		size_t tokenPos = source.find(nameToken, 0);
		{
			size_t eol = source.find_first_of("\r\n", tokenPos);

			size_t begin = tokenPos + nameTokenLength + 1;
			std::string name = source.substr(begin, eol - begin);
			m_specification.name = name;
			tokenPos = std::string::npos;
		}

		{
			tokenPos = source.find(textureCountToken, 0);

			size_t eol = source.find_first_of("\r\n", tokenPos);

			size_t begin = tokenPos + texCountTokenLength + 1;
			std::string name = source.substr(begin, eol - begin);
			m_specification.textureCount = stoi(name);
		}

		{
			tokenPos = source.find(internalShaderToken, 0);
			size_t eol = source.find_first_of("\r\n", tokenPos);

			size_t begin = tokenPos + internalShaderTokenLength + 1;
			std::string text = source.substr(begin, eol - begin);
			if (text == "true")
			{
				m_specification.internalShader = true;
			}
			else
			{
				m_specification.internalShader = false;
			}
		}

		{
			tokenPos = source.find(textureNamesToken, 0);

			size_t eol = source.find_first_of("\r\n", tokenPos);
			eol = source.find_first_of("\r\n", eol + 2);

			for (int i = 0; i < m_specification.textureCount; i++)
			{
				size_t pos = source.find_first_of("\r\n", eol + 2);
				std::string name = source.substr(eol + 2, pos - eol - 2);
				m_specification.inputTextureNames.push_back(name);

				eol = pos;
			}
		}

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
		GLuint program = glCreateProgram();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cacheDir = Utils::GetCacheDirectory();

		auto& shaderData = m_vulkanSPIRV;
		shaderData.clear();

		for (auto&& [stage, source] : sources)
		{
			std::filesystem::path cachePath = cacheDir / (Path.filename().string() + Utils::GLShaderStageCahcedVulkanFileExtension(stage));
		
			std::ifstream in(cachePath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), Path.string().c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					LP_CORE_ERROR(module.GetErrorMessage());
					LP_CORE_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachePath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];

					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, source] : shaderData)
		{
			Reflect(stage, source);
		}
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
		
		const bool optimize = true;
		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}
		
		std::filesystem::path cacheDir = Utils::GetCacheDirectory();

		auto& shaderData = m_openGLSPIRV;
		shaderData.clear();
		m_openGLSource.clear();

		for (auto&& [stage, spirv] : m_vulkanSPIRV)
		{
			std::filesystem::path cachePath = cacheDir / (Path.filename().string() + Utils::GLShaderStageCahcedOpenGLFileExtension(stage));

			std::ifstream in(cachePath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_openGLSource[stage] = glslCompiler.compile();
				auto& source = m_openGLSource[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), Path.string().c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					LP_CORE_ERROR(module.GetErrorMessage());
					LP_CORE_ASSERT(false, "");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachePath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];

					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIds;
		for (auto&& [stage, spirv] : m_openGLSPIRV)
		{
			GLuint shaderId = shaderIds.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderId, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderId);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			LP_CORE_ERROR("Shader link failed ({0}): \n{1}", Path.string(), infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIds)
			{
				glDeleteShader(id);
			}
		}

		for (auto id : shaderIds)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_rendererId = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		LP_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), Path.string());
		LP_CORE_TRACE("		{0} uniform buffers", resources.uniform_buffers.size());
		LP_CORE_TRACE("		{0} resources", resources.sampled_images.size());

		LP_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			LP_CORE_TRACE("		{0}", resource.name);
			LP_CORE_TRACE("			Size = {0}", bufferSize);
			LP_CORE_TRACE("			Binding = {0}", binding);
			LP_CORE_TRACE("			Members = {0}", memberCount);
		}
	}
}
