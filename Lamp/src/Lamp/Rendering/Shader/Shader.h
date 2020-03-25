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
		inline const uint32_t GetID() const { return m_ID; }
		inline const std::string& GetVertexPath() { return m_VertexPath; }
		inline const std::string& GetFragementPath() { return m_FragmentPath; }

	public:
		static std::shared_ptr<Shader> Create(const std::string& vertexPath, const std::string& fragmentPath);

	private:
		uint32_t m_ID;
		std::string m_VertexPath;
		std::string m_FragmentPath;
	};
}