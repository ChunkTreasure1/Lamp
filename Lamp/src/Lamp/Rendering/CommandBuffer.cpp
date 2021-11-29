#include "lppch.h"
#include "CommandBuffer.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Lamp/Rendering/Renderer.h"

namespace Lamp
{
	Scope<CommandBuffer> CommandBuffer::Create(Ref<RenderPipeline> renderPipeline)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::OpenGL:
				break;

			case RendererAPI::API::Vulkan:
				break;
		}
		return Scope<CommandBuffer>();
	}
}