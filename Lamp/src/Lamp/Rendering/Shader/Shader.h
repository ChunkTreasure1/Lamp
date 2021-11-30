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

	enum class UniformType : uint32_t
	{
		Int = 0,
		Float = 1,
		Float2 = 2,
		Float3 = 3,
		Float4 = 4,
		Mat3 = 5,
		Mat4 = 6,
		Sampler2D = 7,
		SamplerCube = 8,
		RenderData = 9
	};

	enum ShaderType
	{
		VertexShader = BIT(1),
		FragmentShader = BIT(2),
		GeometryShader = BIT(3),
		ComputeShader = BIT(4)
	};

	struct UniformSpecification
	{
		UniformSpecification(const std::string& name, UniformType type, std::any data, uint32_t id)
			: name(name), type(type), data(data), id(id)
		{ }

		std::string name;
		UniformType type;
		std::any data;
		uint32_t id;
	};

	struct ShaderSpecification
	{
		int textureCount;
		std::string name;
		std::vector<std::string> textureNames;
		std::vector<UniformSpecification> uniforms;
		int type = 0;
		bool isInternal;
	};

	struct ShaderResourceDeclaration
	{
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(const std::string& name, uint32_t binding, uint32_t x)
			: name(name), binding(binding)
		{
		}

		std::string name;
		uint32_t binding;
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

		inline const ShaderSpecification& GetSpecification() { return m_specification; }

	public:
		static Ref<Shader> Create(const std::string& path);

	protected:
		ShaderSpecification m_specification;
	};
}