#pragma once

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/GraphicsContext.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

namespace Lamp
{
	class VulkanPhysicalDevice;
	class VulkanDevice;
	class Swapchain;

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(GLFWwindow* window);
		~VulkanContext();

		void Initialize() override;
		void SwapBuffers() override {}

		void Shutdown();
		void Update();

		Ref<VulkanDevice> GetDevice() { return m_device; }

		static VkInstance GetInstance() { return s_instance; }
		static Ref<VulkanContext> Get();
		static Ref<VulkanDevice> GetCurrentDevice() { return Get()->GetDevice(); }

	private:
		void CreateInstance();
		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();

		const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

		uint32_t m_width = 1280;
		uint32_t m_height = 720;

		VkDebugUtilsMessengerEXT m_debugMessenger;

		Ref<VulkanPhysicalDevice> m_physicalDevice;
		Ref<VulkanDevice> m_device;

		VmaAllocator m_vulkanAllocator;

		static VkInstance s_instance;
	};
}