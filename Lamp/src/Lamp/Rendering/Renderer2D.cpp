#include "lppch.h"

#include "Renderer2D.h"

#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/RenderPipeline.h"

namespace Lamp
{
	void Renderer2D::Initialize()
	{
		SetupRenderPipelines();
	}

	void Renderer2D::Shutdown()
	{
	}

	void Renderer2D::Begin(const Ref<CameraBase> camera)
	{ 
	}

	void Renderer2D::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, size_t id)
	{
	}

	void Renderer2D::SubmitLine(const glm::vec3& pointOne, const glm::vec3& pointTwo, const glm::vec4& color)
	{ 
	}

	void Renderer2D::SetupRenderPipelines()
	{
		//Quad pass
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::R32UI,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("quad");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.drawSkybox = false;
			pipelineSpec.drawTerrain = false;
			pipelineSpec.debugName = "Quad";
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float4, "a_Color" },
				{ ElementType::Float2, "a_TexCoords" },
				{ ElementType::Int, "a_TexId" },
				{ ElementType::Int, "a_Id" },
			};

			m_storage->quadPipeline = RenderPipeline::Create(pipelineSpec);
		}

		//Line pass
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::R32UI,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("line");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::LineList;
			pipelineSpec.drawSkybox = false;
			pipelineSpec.drawTerrain = false;
			pipelineSpec.debugName = "Line";
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
			};

			m_storage->linePipeline = RenderPipeline::Create(pipelineSpec);
		}
	}
}