#include "lppch.h"
#include "RenderPipelineLibrary.h"

#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Renderer.h"

namespace Lamp
{
	RenderPipelineLibrary* RenderPipelineLibrary::s_instance = nullptr;

	RenderPipelineLibrary::RenderPipelineLibrary()
	{
		LP_CORE_ASSERT(!s_instance, "Instance already exists! Singleton should not be created more than once!")
		s_instance = this;
	
		SetupRenderPipelines();
	}

	RenderPipelineLibrary::~RenderPipelineLibrary()
	{
		s_instance = nullptr;
	}

	Ref<RenderPipeline> RenderPipelineLibrary::GetPipeline(ERenderPipeline pipeline)
	{
		return m_renderPipelines[pipeline];
	}

	ERenderPipeline RenderPipelineLibrary::GetTypeFromPipeline(Ref<RenderPipeline> pipeline)
	{
		for (const auto mPipeline : m_renderPipelines)
		{
			if (mPipeline.second == pipeline)
			{
				return mPipeline.first;
			}
		}
	}

	RenderPipelineLibrary& RenderPipelineLibrary::Get()
	{
		return *s_instance;
	}

	void RenderPipelineLibrary::SetupRenderPipelines()
	{
		//GBuffer
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::RGBA32F,
				ImageFormat::RGBA32F,
				ImageFormat::RGBA32F,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("gbuffer");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.debugName = "GBuffer";
			pipelineSpec.pipelineType = ERenderPipeline::Deferred;
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			m_renderPipelines[ERenderPipeline::Deferred] = RenderPipeline::Create(pipelineSpec);
		}
	}
}