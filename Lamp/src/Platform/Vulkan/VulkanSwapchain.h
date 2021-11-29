#pragma once

#include "Lamp/Rendering/Swapchain.h"

#include <vulkan/vulkan_core.h>
#include <glfw/glfw3.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class VulkanDevice;
	class VulkanSwapchain : public Swapchain
	{
	public:
		VulkanSwapchain(Ref<GraphicsContext> context, void* device);
		~VulkanSwapchain();

		void InitializeSurface(void* window) override;
		void Shutdown() override;
		
		void Invalidate(uint32_t& width, uint32_t& height) override;
		void BeginFrame() override;
		void Present() override;
		void OnResize(uint32_t width, uint32_t height) override;

		void Create(uint32_t& width, uint32_t& height);

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

	private:
		void FindCapabilities();
		void CreateSyncObjects();
		void CreateRenderPass();
		void CreateImageViews();
		void CreateFramebuffer();
		void CreateCommandPools();
		void CreateCommandBuffers();
		void CreateDepthBuffer();

		GLFWwindow* m_pWindow;
		VkExtent2D m_extent;

		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
		std::vector<VkFramebuffer> m_framebuffers;

		VkImage m_depthImage;
		VkImageView m_depthImageView;
		VmaAllocation m_depthImageMemory;

		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;

		VkInstance m_instance;
		VulkanDevice* m_device = nullptr;

		VkSwapchainKHR m_swapchain = nullptr;
		VkSurfaceKHR m_surface = nullptr;
		VkRenderPass m_renderPass = nullptr;

		uint32_t m_queueNodeIndex;
		uint32_t m_currentImageIndex = 0;
		uint32_t m_imageCount = 0;

		VkFormat m_colorFormat;
		VkColorSpaceKHR m_colorSpace;
		VkPresentModeKHR m_presentMode;

		uint32_t m_width;
		uint32_t m_height;

		std::vector<VkSemaphore> m_presentCompleteSemaphores;
		std::vector<VkSemaphore> m_renderCompleteSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		uint32_t m_currentFrame = 0;
	};
}