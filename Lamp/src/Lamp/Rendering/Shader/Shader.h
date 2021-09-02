#pragma once

#include <glad/glad.h>

#include <string>

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Lamp/Utility/Types.h"

namespace Lamp
{
	enum ShaderType
	{
		VertexShader = 0,
		FragmentShader = BIT(1),
		GeometryShader = BIT(2)
	};

	struct ShaderSpec
	{
		std::string Name;
		int TextureCount;
		std::vector<std::string> TextureNames;
		ShaderType Types;
	};

	struct ShaderUniform
	{
		ShaderUniform(const std::string& name, Type type, void* pData)
			: Name(name), Type(type), Data(pData)
		{
			Size = GetSizeFromType(type);
		}

		std::string Name;
		Type Type;
		uint32_t Size;
		void* Data = nullptr;
	};

	struct ShaderData
	{
		ShaderData(std::initializer_list<ShaderUniform> data)
			: Data(data)
		{}

		std::vector<ShaderUniform> Data;
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void UploadData(const ShaderData& data) = 0;

		virtual const std::string& GetName() = 0;
		virtual std::string& GetPath() = 0;

		inline const ShaderSpec GetSpecifications() { return m_Specifications; }

	public:
		static Ref<Shader> Create(const std::string& path);

	protected:

	protected:
		ShaderSpec m_Specifications;
	};
}