#include "lppch.h"
#include "Skybox.h"

#include "Lamp/Rendering/Textures/TextureHDR.h"
#include "Lamp/Rendering/Textures/TextureCube.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"

#include "Lamp/Rendering/RenderCommand.h"

namespace Lamp
{
	Skybox::Skybox(const std::filesystem::path& path)
	{
		m_hdrTexture = TextureHDR::Create(path);

		////Setup shaders
		//m_eqCubeShader = ShaderLibrary::GetShader("EqCube");
		//m_convolutionShader = ShaderLibrary::GetShader("Convolution");
		//m_prefilterShader = ShaderLibrary::GetShader("Prefilter");
		//m_skyboxShader = ShaderLibrary::GetShader("Skybox");

		////Create views/projection
		//m_captureProjection = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 100.f);
		//m_captureViews =
		//{
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		//	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		//};


	}

	Skybox::~Skybox()
	{
	}

	void Skybox::Draw()
	{
		LP_PROFILE_FUNCTION();

		m_cubeMap->Bind(0);
		m_skyboxShader->Bind();
	}

	void Skybox::GenerateBRDFLUT()
	{
		const uint32_t brdfDim = 512;

		ImageSpecification imageSpec{};
		imageSpec.format = ImageFormat::RG16F;
		imageSpec.usage = ImageUsage::Attachment;
		imageSpec.width = brdfDim;
		imageSpec.height = brdfDim;
		imageSpec.filter = TextureFilter::Linear;
		imageSpec.wrap = TextureWrap::Clamp;

		m_brdfLUT = Image2D::Create(imageSpec);


	}
}