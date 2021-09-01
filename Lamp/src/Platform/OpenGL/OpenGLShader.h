#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(std::initializer_list<std::string> paths);
		virtual ~OpenGLShader() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void UploadData(const ShaderData& data) override;

		virtual const std::string& GetName() override { return m_Specifications.Name; }
		virtual std::string& GetFragmentPath() override { return m_FragmentPath; }
		virtual std::string& GetVertexPath() override { return m_VertexPath; }
		virtual std::string& GetGeoPath() override { return m_GeoPath; }

	private:
		uint32_t m_RendererID;
		std::string m_FragmentPath;
		std::string m_VertexPath;
		std::string m_GeoPath;

		std::vector<std::string> m_Paths;
	};
}