#pragma once

#include <glad/glad.h>

#include <string>

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Lamp
{
	enum class ShaderType
	{
		Illum = 0,
		Blinn,
		Phong,
		BlinnPhong,
		Unknown
	};

	enum ShaderDataType : uint16_t
	{
		Bool = 0,
		Int,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		IntArray
	};

	static uint32_t GetSizeFromType(ShaderDataType elementType)
	{
		switch (elementType)
		{
			case Lamp::ShaderDataType::Bool: return 1;
			case Lamp::ShaderDataType::Int: return 4;
			case Lamp::ShaderDataType::Float: return 4;
			case Lamp::ShaderDataType::Float2: return 4 * 2;
			case Lamp::ShaderDataType::Float3: return 4 * 3;
			case Lamp::ShaderDataType::Float4: return 4 * 4;
			case Lamp::ShaderDataType::Mat3: return 4 * 3 * 3;
			case Lamp::ShaderDataType::Mat4: return 4 * 4 * 4;
		}

		return 0;
	}

	struct ShaderUniform
	{
		ShaderUniform(const std::string& name, ShaderDataType type, void* pData)
			: Name(name), Type(type), Data(pData)
		{
			Size = GetSizeFromType(type);
		}
		std::string Name;
		ShaderDataType Type;
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

		inline const ShaderType GetType() { return m_Type; }

	public:
		static Ref<Shader> Create(std::initializer_list<std::string> paths);

	protected:
		ShaderType ShaderTypeFromString(const std::string& s);

	protected:
		ShaderType m_Type;
	};
}