#pragma once

#include <glad/glad.h>

#include <string>

#include <fstream>
#include <sstream>
#include <iostream>

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
		GeometryShader = BIT(3)
	};

	struct ShaderSpec
	{
		std::string Name;
		int TextureCount;
		std::vector<std::string> TextureNames;
		std::map<std::string, UniformType> Uniforms;
		int Type = 0;
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

		inline const ShaderSpec GetSpecifications() { return m_Specifications; }

	public:
		static Ref<Shader> Create(const std::string& path);

	protected:
		ShaderSpec m_Specifications;
	};
}