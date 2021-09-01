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
	struct ShaderSpec
	{
		std::string Name;
		int TextureCount;
		std::vector<std::string> TextureNames;
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
		virtual std::string& GetFragmentPath() = 0;
		virtual std::string& GetVertexPath() = 0;
		virtual std::string& GetGeoPath() = 0;

		inline const ShaderSpec GetSpecifications() { return m_Specifications; }

	public:
		static Ref<Shader> Create(std::initializer_list<std::string> paths);

	protected:

	protected:
		ShaderSpec m_Specifications;
	};
}