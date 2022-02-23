#pragma once

#include "Lamp/Core/Core.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

#include <cstdint>

namespace Lamp
{
	class VulkanDevice;
	class Swapchain
	{
	public:
		using ResizeCallback = std::function<void(uint32_t width, uint32_t height)>;

		struct SwapchainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		Swapchain(void* instance, void* device);
		~Swapchain();

		void InitializeSurface(void* window);
		void Shutdown();
		void BeginFrame();
		void Present();
		void Invalidate(uint32_t& width, uint32_t& height, bool vSync);

		void OnResize(uint32_t width, uint32_t height);
		void Create(uint32_t& width, uint32_t& height);

		inline void RegisterResizeCallback(void* owner, ResizeCallback callback) { LP_CORE_ASSERT(callback, "Callback is nullptr!"); m_resizeCallbacks.emplace(owner, callback); }
		void UnregisterResizeCallback(void* owner);

		inline VkFormat GetFormat() const { return m_colorFormat; }
		inline const VkSwapchainKHR& GetHandle() const { return m_swapchain; }
		inline const VkSurfaceKHR GetSurface() const { return m_surface; }

		inline const size_t GetCommandBufferCount() const { return m_commandBuffers.size(); }
		const uint32_t GetCurrentFrame() const { return m_currentFrame; }
		inline VkCommandBuffer GetDrawCommandBuffer(uint32_t index) { LP_CORE_ASSERT((index < m_commandBuffers.size()) && (index >= 0), "Index out of bounds!") return m_commandBuffers[index]; }
		inline VkCommandPool GetDrawCommandPool(uint32_t index) { LP_CORE_ASSERT((index < m_commandPools.size()) && (index >= 0), "Index out of bounds!"); return m_commandPools[index]; }
		inline VkFramebuffer GetFramebuffer(uint32_t index) { LP_CORE_ASSERT(index < m_framebuffers.size() && index >= 0, "Index out of bounds!"); return m_framebuffers[index]; }

		inline VkRenderPass GetRenderPass() { return m_renderPass; }
		inline VkExtent2D GetExtent() { return m_extent; }
		inline const uint32_t GetImageCount() const { return m_imageCount; }
		inline const uint32_t GetCurrentImage() const { return m_currentImageIndex; }

		inline VkFramebuffer GetCurrentFramebuffer() { return m_framebuffers[m_currentImageIndex]; }

		static Scope<Swapchain> Create(void* instance, void* device);

	private:
		void FindCapabilities();
		void CreateSyncObjects();
		void CreateRenderPass();
		void CreateImageViews();
		void CreateFramebuffer();
		void CreateCommandPools();
		void CreateCommandBuffers();
		void CreateDepthBuffer();

		std::unordered_map<void*, ResizeCallback> m_resizeCallbacks;

		GLFWwindow* m_pWindow;
		VkExtent2D m_extent;

		VkImage m_depthImage;
		VkImageView m_depthImageView;
		VmaAllocation m_depthImageMemory;

		VkInstance m_instance;
		VulkanDevice* m_device;

		VkSwapchainKHR m_swapchain = nullptr;
		VkSurfaceKHR m_surface = nullptr;
		VkRenderPass m_renderPass = nullptr;

		VkFormat m_colorFormat;
		VkColorSpaceKHR m_colorSpace;
		VkPresentModeKHR m_presentMode;
		VkImageUsageFlags m_availibleUsageFlags = 0;

		uint32_t m_queueNodeIndex;
		uint32_t m_imageCount = 0;

		uint32_t m_width;
		uint32_t m_height;

		bool m_vsync = false;

		VkSemaphore m_presentCompleteSemaphore;
		VkSemaphore m_renderCompleteSemaphore;

		uint32_t m_currentFrame = 0;
		uint32_t m_currentImageIndex = 0;

		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
		std::vector<VkFramebuffer> m_framebuffers;
		std::vector<VkFence> m_waitFences;

		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
}