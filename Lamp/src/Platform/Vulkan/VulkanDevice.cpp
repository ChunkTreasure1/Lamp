#include "lppch.h"
#include "VulkanDevice.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include <set>

namespace Lamp
{
	/////Physical device//////
	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{
		auto instance = VulkanContext::GetVulkanInstance();

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find GPU with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices{ deviceCount };
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		//Find device
		for (const auto& device : devices)
		{
			vkGetPhysicalDeviceProperties(device, &m_properties);
			if (m_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				m_physicalDevice = device;
				break;
			}
		}

		LP_CORE_ASSERT(m_physicalDevice, "Unable to find a supported device!");

		//Get properties
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

		Renderer::s_capabilities.maxAniostropy = m_properties.limits.maxSamplerAnisotropy;
		Renderer::s_capabilities.supportAniostopy = m_features.samplerAnisotropy == VK_TRUE ? true : false;

		//Get extensions
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
		LP_CORE_ASSERT(queueFamilyCount > 0, "No queue families supported!");

		m_queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);

		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (const auto& ext : extensions)
				{
					m_supportedExtensions.emplace(ext.extensionName);
				}
			}
		}

		//Queue families
		m_queueFamilyIndices = FindQueueFamilyIndices();
		std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value() };

		float queuePriority = 1.f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueFamilyIndex = queueFamily;
			createInfo.queueCount = 1;
			createInfo.pQueuePriorities = &queuePriority;
			m_queueCreateInfos.push_back(createInfo);
		}
	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{
	}

	const bool VulkanPhysicalDevice::IsExtensionSupported(const std::string& extension) const
	{
		return m_supportedExtensions.find(extension) != m_supportedExtensions.end();
	}

	Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::Create()
	{
		return CreateRef<VulkanPhysicalDevice>();
	}

	VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilyIndices()
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

		m_queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

		uint32_t i = 0;
		for (const auto& queueFamily : m_queueFamilyProperties)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				indices.presentFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;
			}
		}

		return indices;
	}

	/////Device//////
	VulkanDevice::VulkanDevice(Ref<VulkanPhysicalDevice> physicalDevice, VkPhysicalDeviceFeatures enabledFeatures)
		: m_physicalDevice(physicalDevice), m_enabledFeatures(enabledFeatures)
	{
		std::vector<const char*> deviceExtensions;
		LP_CORE_ASSERT(m_physicalDevice->IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME), "Device does not support swapchain extension!");
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(m_physicalDevice->m_queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = m_physicalDevice->m_queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &m_enabledFeatures;

		if (!deviceExtensions.empty())
		{
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

	#ifdef LP_VALIDATION
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();
	#endif

		LP_VK_CHECK(vkCreateDevice(m_physicalDevice->GetHandle(), &deviceCreateInfo, nullptr, &m_logicalDevice));

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = m_physicalDevice->m_queueFamilyIndices.graphicsFamily.value();
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		LP_VK_CHECK(vkCreateCommandPool(m_logicalDevice, &commandPoolInfo, nullptr, &m_commandPool));

		commandPoolInfo.queueFamilyIndex = m_physicalDevice->m_queueFamilyIndices.computeFamily.value();

		LP_VK_CHECK(vkCreateCommandPool(m_logicalDevice, &commandPoolInfo, nullptr, &m_computeCommandPool));

		vkGetDeviceQueue(m_logicalDevice, m_physicalDevice->m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_logicalDevice, m_physicalDevice->m_queueFamilyIndices.computeFamily.value(), 0, &m_computeQueue);
	}

	VulkanDevice::~VulkanDevice()
	{
	}

	void VulkanDevice::Destroy()
	{
		vkDestroyCommandPool(m_logicalDevice, m_computeCommandPool, nullptr);
		vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
		vkDeviceWaitIdle(m_logicalDevice);
		vkDestroyDevice(m_logicalDevice, nullptr);
	}

	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer)
	{
		FlushCommandBuffer(commandBuffer, m_graphicsQueue);
	}

	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		LP_CORE_ASSERT(commandBuffer != VK_NULL_HANDLE, "Command buffer is null!");

		LP_VK_CHECK(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		VkFence fence;
		LP_VK_CHECK(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &fence));
		LP_VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));
		LP_VK_CHECK(vkWaitForFences(m_logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX));

		vkDestroyFence(m_logicalDevice, fence, nullptr);
		vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
	}

	void VulkanDevice::FreeCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
	}

	VkCommandBuffer VulkanDevice::GetCommandBuffer(bool begin, bool compute)
	{
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = compute ? m_computeCommandPool : m_commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		LP_VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &allocateInfo, &commandBuffer));

		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			LP_VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		}

		return commandBuffer;
	}

	VkCommandBuffer VulkanDevice::CreateSecondaryCommandBuffer()
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;

		LP_VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &cmdBuffer));
		return cmdBuffer;
	}

	Ref<VulkanDevice> VulkanDevice::Create(Ref<VulkanPhysicalDevice> physicalDevice, VkPhysicalDeviceFeatures enabledFeatures)
	{
		return CreateRef<VulkanDevice>(physicalDevice, enabledFeatures);
	}
}