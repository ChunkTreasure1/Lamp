#include "lppch.h"
#include "Skybox.h"

#include "Lamp/Rendering/Textures/TextureHDR.h"
#include "Lamp/Rendering/Textures/TextureCube.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"

#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Rendering/Renderer3D.h"

namespace Lamp
{
	Skybox::Skybox(const std::filesystem::path& path)
	{
		if (path.empty() || !std::filesystem::exists(path))
		{
			LP_CORE_ERROR("File {0} not found!", path.string());
			return;
		}
	
		m_hdrTexture = TextureHDR::Create(path);
		if (!m_hdrTexture->IsValid())
		{
			LP_CORE_ERROR("Unable to load texture {0}!", path.string());
			return;
		}

		//Setup shaders
		m_eqCubeShader = ShaderLibrary::GetShader("EqCube");
		m_convolutionShader = ShaderLibrary::GetShader("Convolution");
		m_prefilterShader = ShaderLibrary::GetShader("Prefilter");
		m_skyboxShader = ShaderLibrary::GetShader("Skybox");

		//Create views/projection
		m_captureProjection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 100.f);
		m_captureViews =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};


	}

	Skybox::~Skybox()
	{
	}

	void Skybox::Draw()
	{
		LP_PROFILE_FUNCTION();

		m_cubeMap->Bind(0);
		m_skyboxShader->Bind();

		Renderer3D::DrawCube();
	}
}