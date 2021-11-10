#include "lppch.h"
#include "Skybox.h"

#include "Vertices/VertexBuffer.h"
#include "Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/Textures/IBLBuffer.h"

namespace Lamp
{
	Skybox::Skybox(const std::filesystem::path& path)
	{
		m_shader = ShaderLibrary::GetShader("Skybox");

		m_iblBuffer = CreateRef<IBLBuffer>(path.string());
		Renderer::s_pSceneData->internalFramebuffers.emplace(std::make_pair("Skybox", m_iblBuffer));
	}

	Skybox::~Skybox()
	{
	}

	void Skybox::Render()
	{
		LP_PROFILE_FUNCTION();

		m_shader->Bind();
		m_shader->UploadInt("u_EnvironmentMap", 0);
		m_iblBuffer->Bind();

		Renderer3D::DrawCube();
	}
}