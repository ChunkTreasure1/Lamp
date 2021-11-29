#include "lppch.h"
#include "RenderPipeline.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanRenderPipeline.h"

namespace Lamp
{
	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineSpecification& specification)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:
				
				break;
			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanRenderPipeline>(specification);
				break;

			default:
				break;
		}
	}
}