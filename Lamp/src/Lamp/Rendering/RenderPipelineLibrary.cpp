#include "lppch.h"
#include "RenderPipelineLibrary.h"

#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Renderer.h"

namespace Lamp
{
	std::unordered_map<std::string, Ref<RenderPipeline>> RenderPipelineLibrary::s_renderPipelines;

	void RenderPipelineLibrary::Initialize()
	{
		SetupRenderPipelines();
	}

	void RenderPipelineLibrary::Shutdown()
	{
		s_renderPipelines.clear();
	}

	Ref<RenderPipeline> RenderPipelineLibrary::GetPipeline(const std::string& pipeline)
	{
		if (auto it = s_renderPipelines.find(pipeline); it == s_renderPipelines.end())
		{
			LP_CORE_CRITICAL("Unable to find pipeline {0}!", pipeline);
			return nullptr;
		}

		return s_renderPipelines[pipeline];
	} 

	std::string RenderPipelineLibrary::GetTypeFromPipeline(Ref<RenderPipeline> pipeline)
	{
		for (const auto mPipeline : s_renderPipelines)
		{
			if (mPipeline.second == pipeline)
			{
				return mPipeline.first;
			}
		}

		return "Null";
	}

	std::vector<std::string> RenderPipelineLibrary::GetPipelineNames()
	{
		std::vector<std::string> names;

		for (const auto& it : s_renderPipelines)
		{
			names.emplace_back(it.first);
		}
		return names;
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
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			s_renderPipelines["Deferred"] = RenderPipeline::Create(pipelineSpec);
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

			s_renderPipelines["Forward"] = RenderPipeline::Create(pipelineSpec);
		}
	}
}