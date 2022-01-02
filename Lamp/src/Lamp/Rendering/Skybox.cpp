#include "lppch.h"
#include "Skybox.h"

#include "Lamp/Core/Time/ScopedTimer.h"

#include "Lamp/Rendering/Textures/TextureCube.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/RenderPipeline.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/Renderer.h"

#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Rendering/RendererDataStructures.h"

namespace Lamp
{
	Skybox::Skybox(const std::filesystem::path& path)
	{
		m_hdrTexture = Texture2D::Create(path, false);

		//GenerateBRDFLUT();
		GenerateEquirectangularCube();
		GenerateIrradianceCube();

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

		//m_cubeMap->Bind(0);
		//m_skyboxShader->Bind();
	}

	void Skybox::GenerateBRDFLUT()
	{
		ScopedTimer timer{ "Generate BRDFLUT" };

		const uint32_t brdfDim = 512;

		FramebufferSpecification framebufferSpec{};
		framebufferSpec.swapchainTarget = false;
		framebufferSpec.width = brdfDim;
		framebufferSpec.height = brdfDim;
		framebufferSpec.attachments =
		{
			ImageFormat::RG16F
		};

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
		m_brdfFramebuffer = pipelineSpec.framebuffer;

		pipelineSpec.shader = ShaderLibrary::GetShader("BRDFIntegrate");
		pipelineSpec.isSwapchain = false;
		pipelineSpec.cullMode = CullMode::Front;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.drawType = DrawType::Quad;
		pipelineSpec.uniformBufferSets = Renderer::GetSceneData()->uniformBufferSet;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" }
		};

		auto renderPass = RenderPipeline::Create(pipelineSpec);

		Renderer::BeginPass(renderPass);
		Renderer::SubmitQuad();
		Renderer::EndPass();
	}

	void Skybox::GenerateIrradianceCube()
	{

		ScopedTimer timer{ "Generate irradiance cube" };

		const uint32_t cubemapSize = 64;

		FramebufferSpecification framebufferSpec{};
		framebufferSpec.swapchainTarget = false;
		framebufferSpec.copyable = true;
		framebufferSpec.width = cubemapSize;
		framebufferSpec.height = cubemapSize;
		framebufferSpec.attachments =
		{
			ImageFormat::RGBA16F
		};

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
		auto framebuffer = pipelineSpec.framebuffer;

		pipelineSpec.shader = ShaderLibrary::GetShader("irradianceCube");
		pipelineSpec.isSwapchain = false;
		pipelineSpec.cullMode = CullMode::None;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.drawType = DrawType::Cube;
		pipelineSpec.uniformBufferSets = Renderer::GetSceneData()->uniformBufferSet;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" }
		};

		pipelineSpec.textureCubeInputs =
		{
			{ m_cubeMap, 0, 0 }
		};

		auto renderPass = RenderPipeline::Create(pipelineSpec);

		m_irradianceMap = TextureCube::Create(cubemapSize, cubemapSize);

		m_irradianceMap->StartDataOverride();
		for (uint32_t i = 0; i < 6; i++)
		{
			glm::mat4 mvp = m_perspective * m_viewMatrices[i];
			renderPass->GetSpecification().uniformBufferSets->Get(4, 0, 0)->SetData(&mvp, sizeof(glm::mat4));

			Renderer::BeginPass(renderPass);
			Renderer::SubmitCube();
			Renderer::EndPass();

			m_irradianceMap->SetData(framebuffer->GetColorAttachment(0), i, 0);
		}
		m_irradianceMap->FinishDataOverride();


		//const uint32_t irradianceDim = 64;
		//const uint32_t mips = static_cast<uint32_t>(std::floor(std::log2(irradianceDim))) + 1;

		//FramebufferSpecification framebufferSpec{};
		//framebufferSpec.swapchainTarget = false;
		//framebufferSpec.copyable = true;
		//framebufferSpec.width = irradianceDim;
		//framebufferSpec.height = irradianceDim;
		//framebufferSpec.attachments =
		//{
		//	ImageFormat::RGBA16F
		//};

		//RenderPipelineSpecification pipelineSpec{};
		//pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);

		//pipelineSpec.shader = ShaderLibrary::GetShader("irradianceCube");
		//pipelineSpec.isSwapchain = false;
		//pipelineSpec.cullMode = CullMode::None;
		//pipelineSpec.topology = Topology::TriangleList;
		//pipelineSpec.drawType = DrawType::Cube;
		//pipelineSpec.uniformBufferSets = Renderer::GetSceneData()->uniformBufferSet;
		//pipelineSpec.vertexLayout =
		//{
		//	{ ElementType::Float3, "a_Position" },
		//	{ ElementType::Float3, "a_Normal" },
		//	{ ElementType::Float3, "a_Tangent" },
		//	{ ElementType::Float3, "a_Bitangent" },
		//	{ ElementType::Float2, "a_TexCoords" }
		//};

		//pipelineSpec.textureInputs =
		//{
		//	{ m_hdrTexture, 0, 0 }
		//};

		//auto renderPass = RenderPipeline::Create(pipelineSpec);
		//m_irradianceMap = TextureCube::Create(irradianceDim, irradianceDim);

		//m_irradianceMap->StartDataOverride();

		//for (uint32_t m = 0; m < mips; m++)
		//{
		//	for (uint32_t f = 0; f < 6; f++)
		//	{
		//		glm::mat4 mvp = m_perspective * m_viewMatrices[f];
		//		renderPass->GetSpecification().uniformBufferSets->Get(4, 0, 0)->SetData(&mvp, sizeof(glm::mat4));

		//		Renderer::BeginPass(renderPass);
		//		Renderer::SubmitCube();
		//		Renderer::EndPass();
		//		
		//		const uint32_t newDim = irradianceDim * std::pow(0.5f, m);
		//		renderPass->GetSpecification().framebuffer->Resize(newDim, newDim);

		//		m_irradianceMap->SetData(renderPass->GetSpecification().framebuffer->GetColorAttachment(0), f, m);
		//	}
		//}

		//m_irradianceMap->FinishDataOverride();
	}

	void Skybox::GenerateEquirectangularCube()
	{
		ScopedTimer timer{ "Generate equirectangular cube" };

		const uint32_t cubemapSize = 1024;

		FramebufferSpecification framebufferSpec{};
		framebufferSpec.swapchainTarget = false;
		framebufferSpec.copyable = true;
		framebufferSpec.width = cubemapSize;
		framebufferSpec.height = cubemapSize;
		framebufferSpec.attachments =
		{
			ImageFormat::RGBA16F
		};

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
		auto framebuffer = pipelineSpec.framebuffer;

		pipelineSpec.shader = ShaderLibrary::GetShader("equirectangularCube");
		pipelineSpec.isSwapchain = false;
		pipelineSpec.cullMode = CullMode::None;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.drawType = DrawType::Cube;
		pipelineSpec.uniformBufferSets = Renderer::GetSceneData()->uniformBufferSet;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" }
		};

		pipelineSpec.textureInputs =
		{
			{ m_hdrTexture, 0, 0 }
		};

		auto renderPass = RenderPipeline::Create(pipelineSpec); 
	
		m_cubeMap = TextureCube::Create(cubemapSize, cubemapSize);

		m_cubeMap->StartDataOverride();
		for (uint32_t i = 0; i < 6; i++)
		{
			glm::mat4 mvp = m_perspective * m_viewMatrices[i];
			renderPass->GetSpecification().uniformBufferSets->Get(4, 0, 0)->SetData(&mvp, sizeof(glm::mat4));

			Renderer::BeginPass(renderPass);
			Renderer::SubmitCube();
			Renderer::EndPass();

			m_cubeMap->SetData(framebuffer->GetColorAttachment(0), i, 0);
		}
		m_cubeMap->FinishDataOverride();
	}
}