#pragma once

#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& path);
		virtual ~OpenGLShader() override;

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void UploadData(const ShaderData& data) override;

		virtual const std::string& GetName() override { return m_Specifications.Name; }
		virtual std::string& GetPath() override { return m_FragmentPath; }

	private:
		uint32_t m_RendererID;
		std::string m_FragmentPath;
		std::string m_VertexPath;
		std::string m_GeoPath;

		std::string m_Path;
	};
}