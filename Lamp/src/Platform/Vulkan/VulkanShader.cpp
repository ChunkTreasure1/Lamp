#include "lppch.h"
#include "VulkanShader.h"

#include "VulkanDevice.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>
#include <fstream>

namespace Lamp
{
	namespace Utils
	{
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

		static VkShaderStageFlagBits ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
			{
				return VK_SHADER_STAGE_VERTEX_BIT;
			}
			else if (type == "fragment" || type == "pixel")
			{
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			else if (type == "compute")
			{
				return VK_SHADER_STAGE_COMPUTE_BIT;
			}

			return (VkShaderStageFlagBits)0;
		}

		static std::string StringFromShaderStage(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return "VERTEX_SHADER";
				case VK_SHADER_STAGE_FRAGMENT_BIT: return "FRAGMENT_SHADER";
				case VK_SHADER_STAGE_COMPUTE_BIT: return "COMPUTE_SHADER";
			}

			return "";
		}

		static std::filesystem::path GetCacheDirectory()
		{
			return "engine/shaders/cache/vulkan";
		}

		static const char* GetShaderStageCachedFileExtension(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return ".vertex.cached";
				case VK_SHADER_STAGE_FRAGMENT_BIT: return ".fragment.cached";
				case VK_SHADER_STAGE_COMPUTE_BIT: return ".compute.cached";
			}

			return "";
		}

		static shaderc_shader_kind ShaderStageToShaderC(VkShaderStageFlagBits stage)
		{
			switch (stage)
			{
				case VK_SHADER_STAGE_VERTEX_BIT: return shaderc_vertex_shader;
				case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_fragment_shader;
				case VK_SHADER_STAGE_COMPUTE_BIT: return shaderc_compute_shader;
			}

			LP_CORE_ASSERT(false, "Stage not supported!");
			return static_cast<shaderc_shader_kind>(0);
		}

		static ShaderUniformType SPIRVTypeToShaderType(spirv_cross::SPIRType type)
		{
			switch (type.basetype)
			{
				case spirv_cross::SPIRType::Boolean: return ShaderUniformType::Bool;
				case spirv_cross::SPIRType::Int:
				{
					if (type.vecsize == 1) return ShaderUniformType::Int;
					if (type.vecsize == 2) return ShaderUniformType::Int2;
					if (type.vecsize == 3) return ShaderUniformType::Int3;
					if (type.vecsize == 4) return ShaderUniformType::Int4;
				}
				case spirv_cross::SPIRType::UInt: return ShaderUniformType::UInt;
				case spirv_cross::SPIRType::Float:
				{
					if (type.columns == 3) return ShaderUniformType::Mat3;
					if (type.columns == 4) return ShaderUniformType::Mat4;

					if (type.vecsize == 1) return ShaderUniformType::Float;
					if (type.vecsize == 2) return ShaderUniformType::Float2;
					if (type.vecsize == 3) return ShaderUniformType::Float3;
					if (type.vecsize == 4) return ShaderUniformType::Float4;
				}
			}

			LP_CORE_ASSERT(false, "No uniform type found!");
			return ShaderUniformType::None;
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			if (!std::filesystem::exists(GetCacheDirectory()))
			{
				std::filesystem::create_directories(GetCacheDirectory());
			}
		}
	}

	static std::unordered_map<uint32_t, std::unordered_map<uint32_t, VulkanShader::UniformBuffer*>> s_uniformBuffers;
	static std::unordered_map<uint32_t, std::unordered_map<uint32_t, VulkanShader::StorageBuffer*>> s_storageBuffers;

	VulkanShader::VulkanShader(const std::filesystem::path& path, bool forceCompile)
	{
		Utils::CreateCacheDirectoryIfNeeded();
		this->Path = path;
		m_name = path.stem().string();

		if (!std::filesystem::exists(path))
		{
			SetFlag(AssetFlag::Missing);
			LP_CORE_ERROR("Shader {0} not found!", path.string());
			return;
		}

		Reload(forceCompile);
	}

	VulkanShader::~VulkanShader()
	{
	}

	void VulkanShader::Reload(bool forceCompile)
	{
		std::string source = Utils::ReadFromFile(Path);

		m_shaderSource = PreProcess(source);

		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> shaderData;
		CompileOrGetBinary(shaderData, forceCompile);
		LoadAndCreateShaders(shaderData);
		ReflectAllShaderStages(shaderData);
		CreateDescriptors();
	}

	std::vector<VkDescriptorSetLayout> VulkanShader::GetAllDescriptorSetLayouts()
	{
		std::vector<VkDescriptorSetLayout> result;
		result.reserve(m_descriptorSetLayouts.size());
		for (auto& layout : m_descriptorSetLayouts)
		{
			result.emplace_back(layout);
		}

		return result;
	}

	std::vector<VkPushConstantRange> VulkanShader::GetAllPushConstantRanges()
	{
		std::vector<VkPushConstantRange> result;
		result.reserve(m_pushConstantRanges.size());
		for (uint32_t i = 0; i < m_pushConstantRanges.size(); i++)
		{
			const auto& pushConstantRange = m_pushConstantRanges[i];
			auto& resultRange = result[i];

			resultRange.stageFlags = pushConstantRange.shaderStage;
			resultRange.offset = pushConstantRange.offset;
			resultRange.size = pushConstantRange.size;
		}

		return result;
	}

	std::unordered_map<VkShaderStageFlagBits, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources;

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

	void VulkanShader::CompileOrGetBinary(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData, bool forceCompile)
	{
		auto cacheDirectory = Utils::GetCacheDirectory();

		if (!std::filesystem::exists(cacheDirectory))
		{
			std::filesystem::create_directories(cacheDirectory);
		}

		for (const auto& [stage, source] : m_shaderSource)
		{
			auto extension = Utils::GetShaderStageCachedFileExtension(stage);
			if (!forceCompile)
			{
				std::filesystem::path p = Path;
				auto path = cacheDirectory / (p.filename().string() + extension);

				std::string cachedFilePath = path.string();

				FILE* f;
				errno_t error = fopen_s(&f, cachedFilePath.c_str(), "rb");
				if (!error)
				{
					fseek(f, 0, SEEK_END);
					uint64_t size = ftell(f);
					fseek(f, 0, SEEK_SET);

					outShaderData[stage] = std::vector<uint32_t>(size / sizeof(uint32_t));
					fread(outShaderData[stage].data(), sizeof(uint32_t), outShaderData[stage].size(), f);
					fclose(f);
				}
			}

			if (outShaderData[stage].empty())
			{
				shaderc::Compiler compiler;
				shaderc::CompileOptions options;

				options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
				options.SetWarningsAsErrors();
				options.SetGenerateDebugInfo();

				const bool optimize = false;
				if (optimize)
				{
					options.SetOptimizationLevel(shaderc_optimization_level_performance);
				}

				//Compile shader
				auto& source = m_shaderSource[stage];
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::ShaderStageToShaderC(stage), Path.string().c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					LP_CORE_ERROR(module.GetErrorMessage());
					LP_CORE_ASSERT(false, "Shader compilation failed!");
				}

				const uint8_t* begin = (const uint8_t*)module.cbegin();
				const uint8_t* end = (const uint8_t*)module.cend();
				const ptrdiff_t size = end - begin;

				outShaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				//Cache compiled shader
				auto cachePath = cacheDirectory / (Path.filename().string() + extension);
				std::string cachedFilePath = cachePath.string();

				//TODO: move to use std::ofstream
				FILE* f;
				fopen_s(&f, cachedFilePath.c_str(), "wb");
				fwrite(outShaderData[stage].data(), sizeof(uint32_t), outShaderData[stage].size(), f);
				fclose(f);
			}
		}
	}

	void VulkanShader::LoadAndCreateShaders(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
	{
		auto device = VulkanContext::GetCurrentDevice()->GetHandle();
		m_pipelineShaderStageInfos.clear();

		for (auto [stage, data] : shaderData)
		{
			LP_CORE_ASSERT(data.size(), "No data!");

			VkShaderModuleCreateInfo moduleCreateInfo{};

			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = data.size() * sizeof(uint32_t);
			moduleCreateInfo.pCode = data.data();

			VkShaderModule shaderModule;
			VkResult result = vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule);
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create shader module!");

			VkPipelineShaderStageCreateInfo& shaderStage = m_pipelineShaderStageInfos.emplace_back();
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = stage;
			shaderStage.module = shaderModule;
			shaderStage.pName = "main";
		}
	}

	void VulkanShader::Reflect(VkShaderStageFlagBits stageFlags, const std::vector<uint32_t>& shaderData)
	{
		auto device = VulkanContext::GetCurrentDevice()->GetHandle();

		spirv_cross::Compiler compiler(shaderData);
		auto resources = compiler.get_shader_resources();

		LP_CORE_INFO("Vulkan Shader - Reflect: {0}, {1}", Utils::StringFromShaderStage(stageFlags), Path.string());

		LP_CORE_INFO("Vulkan Shader - Reflect: Uniform buffers");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);

			int memberCount = (uint32_t)bufferType.member_types.size();

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t size = (uint32_t)compiler.get_declared_struct_size(bufferType);

			if (descriptorSet >= m_shaderDescriptorSets.size())
			{
				m_shaderDescriptorSets.resize(descriptorSet + 1);
			}

			ShaderDescriptorSet& shaderDescriptorSet = m_shaderDescriptorSets[descriptorSet];
			if (s_uniformBuffers[descriptorSet].find(binding) == s_uniformBuffers[descriptorSet].end())
			{
				UniformBuffer* uniformBuffer = new UniformBuffer();
				uniformBuffer->bindPoint = binding;
				uniformBuffer->size = size;
				uniformBuffer->name = name;
				uniformBuffer->shaderStage = VK_SHADER_STAGE_ALL;

				s_uniformBuffers.at(descriptorSet)[binding] = uniformBuffer;
			}
			else
			{
				UniformBuffer* uniformBuffer = s_uniformBuffers.at(descriptorSet).at(binding);
				if (size > uniformBuffer->size)
				{
					uniformBuffer->size = size;
				}
			}

			shaderDescriptorSet.uniformBuffers[binding] = s_uniformBuffers.at(descriptorSet).at(binding);
		}

		LP_CORE_INFO("Vulkan Shader - Reflect: Storage buffers");
		for (const auto& resource : resources.storage_buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);

			int memberCount = (uint32_t)bufferType.member_types.size();

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t size = (uint32_t)compiler.get_declared_struct_size(bufferType);

			if (descriptorSet >= m_shaderDescriptorSets.size())
			{
				m_shaderDescriptorSets.resize(descriptorSet + 1);
			}

			ShaderDescriptorSet& shaderDescriptorSet = m_shaderDescriptorSets[descriptorSet];
			if (s_storageBuffers[descriptorSet].find(binding) == s_storageBuffers[descriptorSet].end())
			{
				StorageBuffer* storageBuffer = new StorageBuffer();
				storageBuffer->bindPoint = binding;
				storageBuffer->size = size;
				storageBuffer->name = name;
				storageBuffer->shaderStage = VK_SHADER_STAGE_ALL;

				s_storageBuffers.at(descriptorSet)[binding] = storageBuffer;
			}
			else
			{
				StorageBuffer* storageBuffer = s_storageBuffers.at(descriptorSet).at(binding);
				if (size > storageBuffer->size)
				{
					storageBuffer->size = size;
				}
			}

			shaderDescriptorSet.storageBuffers[binding] = s_storageBuffers.at(descriptorSet).at(binding);
		}

		LP_CORE_INFO("Vulkan Shader - Reflect: Push constants");
		for (const auto& resource : resources.push_constant_buffers)
		{
			const auto& name = resource.name;
			auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);

			uint32_t memberCount = uint32_t(bufferType.member_types.size());
			uint32_t bufferOffset = 0;

			if (!m_pushConstantRanges.empty())
			{
				bufferOffset = m_pushConstantRanges.back().offset + m_pushConstantRanges.back().size;
			}

			auto& pushConstantRange = m_pushConstantRanges.emplace_back();
			pushConstantRange.shaderStage = stageFlags;
			pushConstantRange.size = bufferSize - bufferOffset;
			pushConstantRange.offset = bufferOffset;

			ShaderBuffer& buffer = m_buffers[name];
			buffer.name = name;
			buffer.size = bufferSize - bufferOffset;

			for (uint32_t i = 0; i < memberCount; i++)
			{
				auto type = compiler.get_type(bufferType.member_types[i]);
				const auto& memberName = compiler.get_member_name(bufferType.self, i);
				auto size = (uint32_t)compiler.get_declared_struct_member_size(bufferType, i);
				auto offset = compiler.type_struct_member_offset(bufferType, i);

				std::string uniformName = name + "." + memberName;
				buffer.uniforms[uniformName] = ShaderUniform(uniformName, Utils::SPIRVTypeToShaderType(type), size, offset);
			}
		}

		LP_CORE_INFO("Vulkan Shader - Reflect: Image samplers");
		for (const auto& resource : resources.sampled_images)
		{
			const auto& name = resource.name;
			auto& baseType = compiler.get_type(resource.base_type_id);
			auto& type = compiler.get_type(resource.type_id);

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t dimension = baseType.image.dim;
			uint32_t arraySize = type.array[0];
			if (arraySize == 0)
			{
				arraySize = 1;
			}
			if (descriptorSet >= m_shaderDescriptorSets.size())
			{
				m_shaderDescriptorSets.resize(descriptorSet + 1);
			}

			ShaderDescriptorSet& shaderDescriptorSet = m_shaderDescriptorSets[descriptorSet];
			auto& imageSampler = shaderDescriptorSet.imageSamplers[binding];
			imageSampler.bindPoint = binding;
			imageSampler.descriptorSet = descriptorSet;
			imageSampler.name = name;
			imageSampler.arraySize = arraySize;

			//TODO: fix shader resource declaration
			m_resources[name] = ShaderResourceDeclaration(name, binding, 1);
		}

		LP_CORE_INFO("Vulkan Shader - Reflect: Storage samplers");
		for (const auto& resource : resources.storage_images)
		{
			const auto& name = resource.name;
			auto& baseType = compiler.get_type(resource.base_type_id);
			auto& type = compiler.get_type(resource.type_id);

			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t dimension = baseType.image.dim;

			if (descriptorSet >= m_shaderDescriptorSets.size())
			{
				m_shaderDescriptorSets.resize(descriptorSet + 1);
			}

			ShaderDescriptorSet& shaderDescriptorSet = m_shaderDescriptorSets[descriptorSet];
			auto& imageSampler = shaderDescriptorSet.storageSamplers[binding];
			imageSampler.bindPoint = binding;
			imageSampler.descriptorSet = descriptorSet;
			imageSampler.name = name;
			imageSampler.shaderStage = stageFlags;

			//TODO: fix shader resource declaration
			m_resources[name] = ShaderResourceDeclaration(name, binding, 1);
		}

		LP_CORE_INFO("Vulkan Shader - Reflect result:\n		Uniform buffers: {0}\n		Storage buffers: {1}\n		Image samplers: {2}\n		Storage samplers: {3}\n",
			m_shaderDescriptorSets[0].uniformBuffers.size(), m_shaderDescriptorSets[0].storageBuffers.size(), m_shaderDescriptorSets[0].imageSamplers.size(), m_shaderDescriptorSets[0].storageSamplers.size());
	}

	void VulkanShader::ReflectAllShaderStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
	{
		m_resources.clear();

		for (auto [stage, data] : shaderData)
		{
			Reflect(stage, data);
		}
	}

	void VulkanShader::CreateDescriptors()
	{
		//Pool
		m_descriptorTypes.clear();
		for (uint32_t set = 0; set < m_shaderDescriptorSets.size(); set++)
		{
			auto& descriptorSet = m_shaderDescriptorSets[set];

			if (!descriptorSet.uniformBuffers.empty())
			{
				VkDescriptorPoolSize& typeCount = m_descriptorTypes[set].emplace_back();
				typeCount.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				typeCount.descriptorCount = (uint32_t)descriptorSet.uniformBuffers.size();
			}

			if (!descriptorSet.storageBuffers.empty())
			{
				VkDescriptorPoolSize& typeCount = m_descriptorTypes[set].emplace_back();
				typeCount.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				typeCount.descriptorCount = (uint32_t)descriptorSet.storageBuffers.size();
			}

			if (!descriptorSet.imageSamplers.empty())
			{
				VkDescriptorPoolSize& typeCount = m_descriptorTypes[set].emplace_back();
				typeCount.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				typeCount.descriptorCount = (uint32_t)descriptorSet.imageSamplers.size();
			}

			if (!descriptorSet.storageSamplers.empty())
			{
				VkDescriptorPoolSize& typeCount = m_descriptorTypes[set].emplace_back();
				typeCount.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				typeCount.descriptorCount = (uint32_t)descriptorSet.storageSamplers.size();
			}

			//Layout
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			for (auto& [binding, uniformBuffer] : descriptorSet.uniformBuffers)
			{
				VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = uniformBuffer->shaderStage;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.binding = binding;

				VkWriteDescriptorSet& set = descriptorSet.writeDescriptorSets[uniformBuffer->name];
				set = {};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = 1;
				set.dstBinding = layoutBinding.binding;
			}

			for (auto& [binding, storageBuffer] : descriptorSet.storageBuffers)
			{
				VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = storageBuffer->shaderStage;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.binding = binding;

				LP_CORE_ASSERT(descriptorSet.uniformBuffers.find(binding) == descriptorSet.uniformBuffers.end(), "Binding already present!");

				VkWriteDescriptorSet& set = descriptorSet.writeDescriptorSets[storageBuffer->name];
				set = {};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = 1;
				set.dstBinding = layoutBinding.binding;
			}

			for (auto& [binding, imageSampler] : descriptorSet.imageSamplers)
			{
				VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				layoutBinding.descriptorCount = imageSampler.arraySize;
				layoutBinding.stageFlags = imageSampler.shaderStage;
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBinding.binding = binding;

				LP_CORE_ASSERT(descriptorSet.uniformBuffers.find(binding) == descriptorSet.uniformBuffers.end(), "Binding already present!");
				LP_CORE_ASSERT(descriptorSet.storageBuffers.find(binding) == descriptorSet.storageBuffers.end(), "Binding already present!");

				VkWriteDescriptorSet& set = descriptorSet.writeDescriptorSets[imageSampler.name];
				set = {};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = imageSampler.arraySize;
				set.dstBinding = layoutBinding.binding;
			}

			for (auto& [bindingAndSet, imageSampler] : descriptorSet.storageSamplers)
			{
				VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings.emplace_back();
				layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				layoutBinding.descriptorCount = 1;
				layoutBinding.stageFlags = imageSampler.shaderStage;
				layoutBinding.pImmutableSamplers = nullptr;

				uint32_t binding = bindingAndSet & 0xffffffff;

				layoutBinding.binding = binding;

				LP_CORE_ASSERT(descriptorSet.uniformBuffers.find(binding) == descriptorSet.uniformBuffers.end(), "Binding already present!");
				LP_CORE_ASSERT(descriptorSet.storageBuffers.find(binding) == descriptorSet.storageBuffers.end(), "Binding already present!");
				LP_CORE_ASSERT(descriptorSet.imageSamplers.find(binding) == descriptorSet.imageSamplers.end(), "Binding already present!");

				VkWriteDescriptorSet& set = descriptorSet.writeDescriptorSets[imageSampler.name];
				set = {};
				set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				set.descriptorType = layoutBinding.descriptorType;
				set.descriptorCount = 1;
				set.dstBinding = layoutBinding.binding;
			}

			VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
			descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorLayout.pNext = nullptr;
			descriptorLayout.bindingCount = (uint32_t)layoutBindings.size();
			descriptorLayout.pBindings = layoutBindings.data();

			if (set >= m_descriptorSetLayouts.size())
			{
				m_descriptorSetLayouts.resize((size_t)(set + 1));
			}

			LP_CORE_INFO("VulkanShader::CreateDescriptors - {0}, {1}", m_name, set);

			auto device = VulkanContext::GetCurrentDevice();
			VkResult result = vkCreateDescriptorSetLayout(device->GetHandle(), &descriptorLayout, nullptr, &m_descriptorSetLayouts[set]);
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor set layout!");
		}
	}
}