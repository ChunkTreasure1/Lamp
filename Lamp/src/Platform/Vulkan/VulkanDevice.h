#pragma once

#include "Lamp/Core/Core.h"

#include <optional>
#include <vector>
#include <unordered_set>

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class VulkanPhysicalDevice
	{
	public:
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
		};

		VulkanPhysicalDevice();
		~VulkanPhysicalDevice();

		inline VkPhysicalDevice GetHandle() const { return m_physicalDevice; }
		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_queueFamilyIndices; }

		const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }
		const VkPhysicalDeviceLimits& GetLimits() const { return m_properties.limits; }

		const bool IsExtensionSupported(const std::string& extension) const;

		static Ref<VulkanPhysicalDevice> Create();

	private:
		friend class VulkanDevice;

		QueueFamilyIndices FindQueueFamilyIndices();

		QueueFamilyIndices m_queueFamilyIndices;

		VkPhysicalDevice m_physicalDevice = nullptr;
		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceFeatures m_features;

		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		std::unordered_set<std::string> m_supportedExtensions;
		std::vector<VkDeviceQueueCreateInfo> m_queueCreateInfos;
	};

	class VulkanDevice
	{
	public:
		VulkanDevice(Ref<VulkanPhysicalDevice> physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);
		~VulkanDevice();

		inline VkQueue GetGraphicsQueue() { return m_graphicsQueue; }
		inline const Ref<VulkanPhysicalDevice> GetPhysicalDevice() const { return m_physicalDevice; }
		inline VkDevice GetHandle() const { return m_logicalDevice; }

		void Destroy();

		void FlushCommandBuffer(VkCommandBuffer commandBuffer);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

		VkCommandBuffer GetCommandBuffer(bool begin);
		VkCommandBuffer CreateSecondaryCommandBuffer();

		static Ref<VulkanDevice> Create(Ref<VulkanPhysicalDevice> physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);

	private:
		VkDevice m_logicalDevice = nullptr;
		Ref<VulkanPhysicalDevice> m_physicalDevice;
		VkPhysicalDeviceFeatures m_enabledFeatures;
		VkCommandPool m_commandPool;

		const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

		VkQueue m_graphicsQueue;
	};
}