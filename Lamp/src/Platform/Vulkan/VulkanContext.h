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
		void* GetInstance() override { return s_instance; }
		void* GetDevice() override { return m_device.get(); }

		void Shutdown() override;
		void Update() override;

		static VkInstance GetVulkanInstance() { return s_instance; }
		static Ref<VulkanContext> Get();
		static VulkanDevice* GetCurrentDevice() { return static_cast<VulkanDevice*>(Get()->GetDevice()); }

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