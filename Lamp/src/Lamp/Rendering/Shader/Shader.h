#pragma once

#include "Lamp/AssetSystem/Asset.h"

#include <string>

#include <fstream>
#include <sstream>
#include <iostream>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

namespace Lamp
{
	enum class ShaderUniformType
	{
		None = 0,
		Bool,
		Int,
		UInt,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Int2,
		Int3,
		Int4
	};

	struct ShaderResourceDeclaration
	{
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(const std::string& name, uint32_t binding, uint32_t set)
			: name(name), binding(binding), set(set)
		{
		}

		std::string name;
		uint32_t binding;
		uint32_t set;
	};

	struct ShaderSpecification
	{
		std::string name;
		std::vector<std::string> inputTextureNames;
		uint32_t textureCount = 0;
		bool internalShader = true;
	};

	class Shader : public Asset
	{
	public:
		struct ShaderUniform
		{
		public:
			ShaderUniform() = default;
			ShaderUniform(std::string name, ShaderUniformType type, uint32_t size, uint32_t offset);

			const std::string& GetName() const { return m_name; }
			const uint32_t GetSize() const { return m_size; }
			const uint32_t GetOffset() const { return m_offset; }
			ShaderUniformType GetType() { return m_type; }

			static std::string UniformTypeToString(ShaderUniformType type);
		private:
			std::string m_name;
			ShaderUniformType m_type = ShaderUniformType::None;
			uint32_t m_size = 0;
			uint32_t m_offset = 0;
		};

		struct ShaderBuffer
		{
			std::string name;
			uint32_t size = 0;
			std::unordered_map<std::string, ShaderUniform> uniforms;
		};

		virtual ~Shader() = default;

		virtual void Reload(bool forceCompile) = 0;
		virtual void Bind() = 0;
		virtual const std::string& GetName() = 0;
		virtual const ShaderSpecification& GetSpecification() const = 0;
		
		static AssetType GetStaticType() { return AssetType::Shader; }
		AssetType GetType() override { return GetStaticType(); }

	public:
		static Ref<Shader> Create(const std::filesystem::path& path, bool forceCompile = false);
	};
}