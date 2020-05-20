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

	class Shader
	{
	public:
		Shader(const std::string& vertexPath, const std::string& fragmentPath);

		void Bind();
		void UploadBool(const std::string& name, bool value) const;
		void UploadInt(const std::string& name, int value) const;
		void UploadFloat(const std::string& name, float value) const;
		void UploadFloat3(const std::string& name, const glm::vec3& value) const;
		void UploadFloat4(const std::string& name, const glm::vec4& value) const;

		void UploadMat4(const std::string& name, const glm::mat4& mat);
		void UploadMat3(const std::string& name, const glm::mat3& mat);

		inline const uint32_t GetID() const { return m_ID; }
		inline std::string& GetVertexPath() { return m_VertexPath; }
		inline const std::string& GetFragementPath() { return m_FragmentPath; }
		inline const ShaderType GetType() { return m_Type; }

	public:
		static std::shared_ptr<Shader> Create(const std::string& vertexPath, const std::string& fragmentPath);

	private:
		ShaderType ShaderTypeFromString(const std::string& s);

	private:
		uint32_t m_ID;
		std::string m_VertexPath;
		std::string m_FragmentPath;
		ShaderType m_Type;
	};
}