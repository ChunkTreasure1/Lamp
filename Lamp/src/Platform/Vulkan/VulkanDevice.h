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
			std::optional<uint32_t> computeFamily;
			std::optional<uint32_t> presentFamily;
		};

		VulkanPhysicalDevice();
		~VulkanPhysicalDevice();

		inline VkPhysicalDevice GetHandle() const { return m_physicalDevice; }
		inline const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_queueFamilyIndices; }

		inline const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }
		inline const VkPhysicalDeviceFeatures& GetFeatures() const { return m_features; }
		inline const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return m_memoryProperties; }
		inline const VkPhysicalDeviceLimits& GetLimits() const { return m_properties.limits; }

		const bool IsExtensionSupported(const std::string& extension) const;

		static Ref<VulkanPhysicalDevice> Create();

	private:
		friend class VulkanDevice;

		QueueFamilyIndices FindQueueFamilyIndices();

		QueueFamilyIndices m_queueFamilyIndices;

		VkPhysicalDevice m_physicalDevice = nullptr;
		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceFeatures m_features;
		VkPhysicalDeviceMemoryProperties m_memoryProperties;

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
		inline VkQueue GetComputeQueue() { return m_computeQueue; }

		inline const Ref<VulkanPhysicalDevice> GetPhysicalDevice() const { return m_physicalDevice; }
		inline VkDevice GetHandle() const { return m_logicalDevice; }
		inline VkCommandPool GetCommandPool() const { return m_commandPool; }
		inline const std::mutex& GetFlushMutex() const { return m_flushMutex; }

		void Destroy();

		void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
		void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free);

		VkCommandBuffer GetCommandBuffer(bool begin, bool compute = false);
		VkCommandBuffer CreateSecondaryCommandBuffer(bool begin);

		static Ref<VulkanDevice> Create(Ref<VulkanPhysicalDevice> physicalDevice, VkPhysicalDeviceFeatures enabledFeatures);

	private:
		VkDevice m_logicalDevice = nullptr;
		Ref<VulkanPhysicalDevice> m_physicalDevice;
		VkPhysicalDeviceFeatures m_enabledFeatures;

		VkCommandPool m_commandPool;
		VkCommandPool m_computeCommandPool;

		const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

		VkQueue m_graphicsQueue;
		VkQueue m_computeQueue;

		std::mutex m_flushMutex;
	};
}