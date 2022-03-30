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

		m_renderPipelinesNameMap["Deferred"] = ERenderPipeline::Deferred;
		m_renderPipelinesNameMap["Forward"] = ERenderPipeline::Forward;
		m_renderPipelinesNameMap["Transparent"] = ERenderPipeline::Transparent;
	}

	RenderPipelineLibrary::~RenderPipelineLibrary()
	{
		s_instance = nullptr;
	}

	Ref<RenderPipeline> RenderPipelineLibrary::GetPipeline(ERenderPipeline pipeline)
	{
		return m_renderPipelines[pipeline];
	}

	Ref<RenderPipeline> RenderPipelineLibrary::GetPipeline(const std::string& pipeline)
	{
		auto it = m_renderPipelinesNameMap.find(pipeline);
		if (it != m_renderPipelinesNameMap.end())
		{
			return m_renderPipelines[m_renderPipelinesNameMap[pipeline]];
		}
		
		LP_CORE_ERROR("Render pipeline {0} not found! Defaulting to deferred!", pipeline);
		return m_renderPipelines[ERenderPipeline::Deferred];
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

	std::vector<std::string> RenderPipelineLibrary::GetPipelineNames() const
	{
		std::vector<std::string> names;

		for (const auto& it : m_renderPipelinesNameMap)
		{
			names.emplace_back(it.first);
		}
		return names;
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

		//Forward
		{
			FramebufferSpecification framebufferSpec;
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.clearColor = { 0.1f, 0.1f, 0.1f, 1.f };
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.framebuffer;

			pipelineSpec.shader = ShaderLibrary::GetShader("pbrForward");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.drawType = DrawType::Opaque;
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.shaderStorageBufferSets = Renderer::Get().GetStorage().shaderStorageBufferSet;
			pipelineSpec.debugName = "Forward";
			pipelineSpec.pipelineType = ERenderPipeline::Forward;
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			pipelineSpec.framebufferInputs =
			{
				{ Renderer::Get().GetDefaults().brdfFramebuffer->GetColorAttachment(0), 0, 7 }
			};

			m_renderPipelines[ERenderPipeline::Forward] = RenderPipeline::Create(pipelineSpec);
		}
	}
}