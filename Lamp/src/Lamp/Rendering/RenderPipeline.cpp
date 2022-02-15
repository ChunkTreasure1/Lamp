#include "lppch.h"
#include "RenderPipeline.h"

#include "Platform/Vulkan/VulkanRenderPipeline.h"
#include "Platform/Vulkan/VulkanRenderComputePipeline.h"

namespace Lamp
{
	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineSpecification& specification)
	{
		return CreateRef<VulkanRenderPipeline>(specification);
	}

	Ref<RenderComputePipeline> RenderComputePipeline::Create(Ref<Shader> computeShader)
	{
		return CreateRef<VulkanRenderComputePipeline>(computeShader);
	}
}