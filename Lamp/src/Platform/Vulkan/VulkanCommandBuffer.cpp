#include "lppch.h"
#include "VulkanCommandBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"

#include "Lamp/Core/Application.h"

namespace Lamp
{
	VulkanCommandBuffer::VulkanCommandBuffer(Ref<RenderPipeline> renderPipeline)
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto swapchain = std::dynamic_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());

		uint32_t framesInFlight = Renderer::GetCapabilities().framesInFlight;

		m_commandBuffers.resize(framesInFlight);
		for (uint32_t frame = 0; frame < framesInFlight; frame++)
		{
			m_commandBuffers[frame] = swapchain->GetDrawCommandBuffer(frame);
		}
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
	}

	void VulkanCommandBuffer::Begin()
	{
		uint32_t frame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		VkResult result = vkResetCommandBuffer(m_commandBuffers[frame], 0);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to reset command buffer!");

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkCommandBuffer commandBuffer = m_commandBuffers[frame];
		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to begin command buffer!");
	}

	void VulkanCommandBuffer::End()
	{
		uint32_t frame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
		VkResult result = vkEndCommandBuffer(m_commandBuffers[frame]);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to end command buffer!");
	}

	void* VulkanCommandBuffer::GetCurrentCommandBuffer()
	{
		uint32_t frame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
		return m_commandBuffers[frame];
	}
}