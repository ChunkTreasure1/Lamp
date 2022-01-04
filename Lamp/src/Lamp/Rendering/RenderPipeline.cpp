#include "lppch.h"
#include "RenderPipeline.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanRenderPipeline.h"
#include "Platform/Vulkan/VulkanRenderComputePipeline.h"

namespace Lamp
{
	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:
				break;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanRenderPipeline>(specification);
		}

		return nullptr;
	}

	Ref<RenderComputePipeline> RenderComputePipeline::Create(Ref<Shader> computeShader)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:
				break;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanRenderComputePipeline>(computeShader);
		}

		return nullptr;
	}
}