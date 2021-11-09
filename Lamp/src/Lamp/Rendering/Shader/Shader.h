#pragma once

#include <glad/glad.h>

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

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Recompile() = 0;

		virtual void UploadBool(const std::string& name, bool value) const = 0;
		virtual void UploadInt(const std::string& name, int value) const = 0;
		virtual void UploadFloat(const std::string& name, float value) const = 0;
		virtual void UploadFloat2(const std::string& name, const glm::vec2& value) const = 0;
		virtual void UploadFloat3(const std::string& name, const glm::vec3& value) const = 0;
		virtual void UploadFloat4(const std::string& name, const glm::vec4& value) const = 0;
		
		virtual void UploadMat4(const std::string& name, const glm::mat4& mat) = 0;
		virtual void UploadMat3(const std::string& name, const glm::mat3& mat) = 0;
		virtual void UploadIntArray(const std::string& name, int* values, uint32_t count) const = 0;

		virtual const std::string& GetName() = 0;
		virtual std::string& GetPath() = 0;

		inline const ShaderSpecification& GetSpecification() { return m_specification; }

	public:
		static Ref<Shader> Create(const std::string& path);

	protected:
		ShaderSpecification m_specification;
	};
}