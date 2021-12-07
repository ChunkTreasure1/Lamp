#pragma once

#include "Lamp/Rendering/Swapchain.h"

#include "VulkanMemoryAllocator/VulkanMemoryAllocator.h"
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

namespace Lamp
{
	class VulkanDevice;
	class VulkanSwapchain : public Swapchain
	{
	public:
		using ResizeCallback = std::function<void(uint32_t width, uint32_t height)>;

		struct SwapchainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		VulkanSwapchain(void* instance, void* device);
		~VulkanSwapchain() override;

		void InitializeSurface(void* window) override;
		void Shutdown() override;
		void BeginFrame() override;
		void Present() override;
		void Invalidate(uint32_t& width, uint32_t& height) override;

		void OnResize(uint32_t width, uint32_t height) override;
		void Create(uint32_t& width, uint32_t& height);

		inline void RegisterResizeCallback(void* owner, ResizeCallback callback) { LP_CORE_ASSERT(callback, "Callback is nullptr!"); m_resizeCallbacks.emplace(owner, callback); }
		void UnregisterResizeCallback(void* owner);

		inline VkFormat GetFormat() const { return m_colorFormat; }
		inline const VkSwapchainKHR& GetHandle() const { return m_swapchain; }
		inline const VkSurfaceKHR GetSurface() const { return m_surface; }

		inline const size_t GetCommandBufferCount() const { return m_commandBuffers.size(); }
		const uint32_t GetCurrentFrame() const override { return m_currentFrame; }
		inline VkCommandBuffer GetDrawCommandBuffer(uint32_t index) { LP_CORE_ASSERT((index < m_commandBuffers.size()) && (index >= 0), "Index out of bounds!") return m_commandBuffers[index]; }
		inline VkCommandPool GetDrawCommandPool(uint32_t index) { LP_CORE_ASSERT((index < m_commandPools.size()) && (index >= 0), "Index out of bounds!"); return m_commandPools[index]; }
		inline VkFramebuffer GetFramebuffer(uint32_t index) { LP_CORE_ASSERT(index < m_framebuffers.size() && index >= 0, "Index out of bounds!"); return m_framebuffers[index]; }

		inline VkRenderPass GetRenderPass() { return m_renderPass; }
		inline VkExtent2D GetExtent() { return m_extent; }
		inline const uint32_t GetImageCount() const { return m_imageCount; }
		inline const uint32_t GetCurrentImage() const { return m_currentImageIndex; }

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

		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
		std::vector<VkFramebuffer> m_framebuffers;

		VkImage m_depthImage;
		VkImageView m_depthImageView;
		VmaAllocation m_depthImageMemory;

		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;

		VkInstance m_instance;
		VulkanDevice* m_device;

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