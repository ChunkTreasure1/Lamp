#include "lppch.h"
#include "Swapchain.h"

#include "Platform/Vulkan/VulkanRenderer.h"

#include "Platform/Vulkan/VulkanUtility.h"

#include <glfw/glfw3.h>

namespace Lamp
{
	namespace Utils
	{
		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
		{
			for (const auto& availableFormat : availableFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
		{
			for (const auto& availablePresentMode : availablePresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
		{
			if (capabilities.currentExtent.width != UINT32_MAX)
			{
				return capabilities.currentExtent;
			}
			else
			{
				int width, height;
				glfwGetFramebufferSize(window, &width, &height);

				VkExtent2D actualExtent = {
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};

				actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actualExtent;
			}
		}

		static Swapchain::SwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
		{
			Swapchain::SwapchainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

			if (formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

	}

	static const std::vector<const char*> s_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	Scope<Swapchain> Swapchain::Create(void* instance, void* device)
	{
		return CreateScope<Swapchain>(static_cast<VkInstance>(instance), static_cast<VulkanDevice*>(device));
	}

	Swapchain::Swapchain(VkInstance instance, VulkanDevice* device)
		: m_instance(instance), m_device(device)
	{
	}

	Swapchain::~Swapchain()
	{
	}

	void Swapchain::FindCapabilities()
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->GetPhysicalDevice()->GetHandle(), m_surface, &capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GetPhysicalDevice()->GetHandle(), m_surface, &formatCount, nullptr);

		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GetPhysicalDevice()->GetHandle(), m_surface, &formatCount, formats.data());

		LP_CORE_ASSERT(!formats.empty(), "No formats found!");

		bool found = false;
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				m_colorFormat = format.format;
				m_colorSpace = format.colorSpace;
				found = true;
				break;
			}
		}

		if (!found)
		{
			m_colorFormat = formats[0].format;
			m_colorSpace = formats[0].colorSpace;
		}
	}

	void Swapchain::CreateSyncObjects()
	{
		uint32_t framesInFlight = Renderer::Get().GetCapabilities().framesInFlight;

		m_presentCompleteSemaphores.resize(framesInFlight);
		m_renderCompleteSemaphores.resize(framesInFlight);
		m_inFlightFences.resize(framesInFlight);
		m_imagesInFlight.resize(m_images.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < framesInFlight; i++)
		{
			if (vkCreateSemaphore(m_device->GetHandle(), &semaphoreInfo, nullptr, &m_presentCompleteSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device->GetHandle(), &semaphoreInfo, nullptr, &m_renderCompleteSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_device->GetHandle(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create synchronization objects for a frame!");
			}
		}
	}

	void Swapchain::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_colorFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = VK_FORMAT_D32_SFLOAT;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		LP_VK_CHECK(vkCreateRenderPass(m_device->GetHandle(), &renderPassInfo, nullptr, &m_renderPass));
	}

	void Swapchain::CreateImageViews()
	{
		m_imageViews.resize(m_images.size());

		for (size_t i = 0; i < m_images.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_colorFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			LP_VK_CHECK(vkCreateImageView(m_device->GetHandle(), &createInfo, nullptr, &m_imageViews[i]));
		}
	}

	void Swapchain::CreateFramebuffer()
	{
		m_framebuffers.resize(m_imageViews.size());

		for (size_t i = 0; i < m_imageViews.size(); i++)
		{
			VkImageView attachments[] =
			{
				m_imageViews[i],
				m_depthImageView
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_extent.width;
			framebufferInfo.height = m_extent.height;
			framebufferInfo.layers = 1;

			LP_VK_CHECK(vkCreateFramebuffer(m_device->GetHandle(), &framebufferInfo, nullptr, &m_framebuffers[i]));
		}
	}

	void Swapchain::CreateCommandPools()
	{
		VulkanPhysicalDevice::QueueFamilyIndices queueFamilyIndices = Utility::FindQueueFamilies(m_device->GetPhysicalDevice()->GetHandle(), m_surface);

		m_commandPools.resize(m_framebuffers.size());

		for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			LP_VK_CHECK(vkCreateCommandPool(m_device->GetHandle(), &poolInfo, nullptr, &m_commandPools[i]));
		}
	}

	void Swapchain::CreateCommandBuffers()
	{
		m_commandBuffers.resize(m_framebuffers.size());

		for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_commandPools[i];
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			LP_VK_CHECK(vkAllocateCommandBuffers(m_device->GetHandle(), &allocInfo, &m_commandBuffers[i]));
		}
	}

	void Swapchain::CreateDepthBuffer()
	{
		m_depthImageMemory = Utility::CreateImage(m_extent.width, m_extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_depthImage);
		m_depthImageView = Utility::CreateImageView(m_depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, m_device);
	}

	void Swapchain::InitializeSurface(void* window)
	{
		LP_VK_CHECK(glfwCreateWindowSurface(m_instance, static_cast<GLFWwindow*>(window), nullptr, &m_surface));

		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_device->GetPhysicalDevice()->GetHandle(), &queueCount, nullptr);

		std::vector<VkQueueFamilyProperties> properties(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_device->GetPhysicalDevice()->GetHandle(), &queueCount, properties.data());

		std::vector<VkBool32> supportsPresentation(queueCount);
		for (uint32_t i = 0; i < queueCount; i++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(m_device->GetPhysicalDevice()->GetHandle(), i, m_surface, &supportsPresentation[i]);
		}

		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < queueCount; i++)
		{
			if ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
				{
					graphicsQueueNodeIndex = i;
				}

				if (supportsPresentation[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		if (presentQueueNodeIndex == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queueCount; i++)
			{
				if (supportsPresentation[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		LP_CORE_ASSERT(graphicsQueueNodeIndex != UINT32_MAX, "No graphics queues found!");
		LP_CORE_ASSERT(presentQueueNodeIndex != UINT32_MAX, "No presentation queues found!");

		m_queueNodeIndex = graphicsQueueNodeIndex;

		FindCapabilities();
	}

	void Swapchain::Shutdown()
	{
		vkDeviceWaitIdle(m_device->GetHandle());

		for (auto& semaphore : m_presentCompleteSemaphores)
		{
			vkDestroySemaphore(m_device->GetHandle(), semaphore, nullptr);
		}

		for (auto& semaphore : m_renderCompleteSemaphores)
		{
			vkDestroySemaphore(m_device->GetHandle(), semaphore, nullptr);
		}

		for (auto& fence : m_inFlightFences)
		{
			vkDestroyFence(m_device->GetHandle(), fence, nullptr);
		}

		for (auto& framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(m_device->GetHandle(), framebuffer, nullptr);
		}

		for (auto& view : m_imageViews)
		{
			vkDestroyImageView(m_device->GetHandle(), view, nullptr);
		}


		VulkanAllocator allocator;
		allocator.DestroyImage(m_depthImage, m_depthImageMemory);

		vkDestroyRenderPass(m_device->GetHandle(), m_renderPass, nullptr);
		vkDestroySwapchainKHR(m_device->GetHandle(), m_swapchain, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}

	void Swapchain::BeginFrame()
	{
		LP_PROFILE_FUNCTION();

		vkWaitForFences(m_device->GetHandle(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
		LP_VK_CHECK(vkResetCommandPool(m_device->GetHandle(), m_commandPools[m_currentFrame], 0));
		LP_VK_CHECK(vkAcquireNextImageKHR(m_device->GetHandle(), m_swapchain, UINT64_MAX, m_presentCompleteSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentImageIndex));

		if (m_imagesInFlight[m_currentImageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_device->GetHandle(), 1, &m_imagesInFlight[m_currentImageIndex], VK_TRUE, UINT64_MAX);
		}
		m_imagesInFlight[m_currentImageIndex] = m_inFlightFences[m_currentFrame];
	}

	void Swapchain::Present()
	{
		LP_PROFILE_FUNCTION();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_presentCompleteSemaphores[m_currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffers[m_currentImageIndex];

		VkSemaphore signalSemaphores[] = { m_renderCompleteSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device->GetHandle(), 1, &m_inFlightFences[m_currentFrame]);

		LP_VK_CHECK(vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]));

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &m_currentImageIndex;

		LP_VK_CHECK(vkQueuePresentKHR(m_device->GetGraphicsQueue(), &presentInfo));

		m_currentFrame = (m_currentFrame + 1) % Renderer::Get().GetCapabilities().framesInFlight;
	}

	void Swapchain::Invalidate(uint32_t& width, uint32_t& height)
	{
		Create(width, height);

		CreateImageViews();
		CreateRenderPass();
		CreateDepthBuffer();
		CreateFramebuffer();
		CreateCommandPools();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void Swapchain::OnResize(uint32_t width, uint32_t height)
	{
		auto device = m_device->GetHandle();

		vkDeviceWaitIdle(device);
		Create(width, height);

		for (auto& framebuffer : m_framebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		VulkanAllocator allocator;
		allocator.DestroyImage(m_depthImage, m_depthImageMemory);

		CreateImageViews();
		CreateDepthBuffer();
		CreateFramebuffer();

		m_currentFrame = 0;
		m_currentImageIndex = 0;

		vkDeviceWaitIdle(device);

		for (const auto& callback : m_resizeCallbacks)
		{
			callback.second(width, height);
		}
	}

	void Swapchain::Create(uint32_t& width, uint32_t& height)
	{
		SwapchainSupportDetails swapChainSupport = Utils::QuerySwapChainSupport(m_device->GetPhysicalDevice()->GetHandle(), m_surface);

		VkSurfaceFormatKHR surfaceFormat = Utils::ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = Utils::ChooseSwapPresentMode(swapChainSupport.presentModes);
		m_extent = Utils::ChooseSwapExtent(swapChainSupport.capabilities, m_pWindow);

		m_imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && m_imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			m_imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainKHR oldSwapchain = m_swapchain;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;

		createInfo.minImageCount = m_imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = m_extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.oldSwapchain = oldSwapchain;

		VulkanPhysicalDevice::QueueFamilyIndices indices = Utility::FindQueueFamilies(m_device->GetPhysicalDevice()->GetHandle(), m_surface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(m_device->GetHandle(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		if (oldSwapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(m_device->GetHandle(), oldSwapchain, nullptr);

			for (auto& view : m_imageViews)
			{
				vkDestroyImageView(m_device->GetHandle(), view, nullptr);
			}

			for (auto& framebuffer : m_framebuffers)
			{
				vkDestroyFramebuffer(m_device->GetHandle(), framebuffer, nullptr);
				framebuffer = nullptr;
			}
		}

		vkGetSwapchainImagesKHR(m_device->GetHandle(), m_swapchain, &m_imageCount, nullptr);
		m_images.resize(m_imageCount);
		vkGetSwapchainImagesKHR(m_device->GetHandle(), m_swapchain, &m_imageCount, m_images.data());

		m_colorFormat = surfaceFormat.format;
	}

	void Swapchain::UnregisterResizeCallback(void* owner)
	{
		LP_CORE_ASSERT(owner, "Owner is nullptr!");

		auto it = m_resizeCallbacks.find(owner);
		if (it != m_resizeCallbacks.end())
		{
			m_resizeCallbacks.erase(owner);
		}
	}
}