#include "lppch.h"

#include "Lamp/Core/Application.h"

#include "Platform/Vulkan/VulkanUtility.h"

#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanAllocator.h"

namespace Lamp
{
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkInstance VulkanContext::s_instance = nullptr;

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				LP_CORE_TRACE("Validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				LP_CORE_INFO("Validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				LP_CORE_WARN("Validation layer: {0}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				LP_CORE_ERROR("Validation layer: {0}", pCallbackData->pMessage);
				break;
		}

		return VK_FALSE;
	}

	namespace VulkanUtils
	{
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

			if (func != nullptr)
			{
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			}
			else
			{
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}

		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			if (func != nullptr)
			{
				func(instance, debugMessenger, pAllocator);
			}
		}
	}

	VulkanContext::VulkanContext(GLFWwindow* window)
	{
	}

	VulkanContext::~VulkanContext()
	{
	}

	void VulkanContext::Initialize()
	{
		CreateInstance();
		SetupDebugMessenger();

		m_physicalDevice = VulkanPhysicalDevice::Create();

		VkPhysicalDeviceFeatures enabledFeatures{};
		enabledFeatures.samplerAnisotropy = VK_TRUE;
		enabledFeatures.pipelineStatisticsQuery = VK_TRUE;
		enabledFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
		enabledFeatures.tessellationShader = VK_TRUE;

		m_device = VulkanDevice::Create(m_physicalDevice, enabledFeatures);

		VulkanAllocator::Initialize(m_device.get());
	}

	void VulkanContext::Shutdown()
	{
		vkDeviceWaitIdle(m_device->GetHandle());

		VulkanAllocator::Shutdown();

	#ifdef LP_VALIDATION
		VulkanUtils::DestroyDebugUtilsMessengerEXT(s_instance, m_debugMessenger, nullptr);
	#endif

		m_device->Destroy();
		vkDestroyInstance(s_instance, nullptr);
	}

	void VulkanContext::Update()
	{
	}

	Ref<VulkanContext> VulkanContext::Get()
	{
		return std::reinterpret_pointer_cast<VulkanContext>(Application::Get().GetWindow().GetGraphicsContext());
	}

	void VulkanContext::CreateInstance()
	{
	#ifdef LP_VALIDATION	
		if (!CheckValidationLayerSupport())
		{
			LP_CORE_ASSERT(false, "Validation layers are enabled but none are supported!");
			return;
		}
	#endif

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Lamp";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		auto extensions = GetRequiredExtensions();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	#ifdef LP_VALIDATION
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	#else
		createInfo.pNext = nullptr;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	#endif

		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
		LP_CORE_ERROR("Vulkan Error {0}", VKResultToString(result));

		s_instance = m_instance;
	}

	void VulkanContext::SetupDebugMessenger()
	{
	#ifndef LP_VALIDATION
		return;
	#endif
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		LP_VK_CHECK(VulkanUtils::CreateDebugUtilsMessengerEXT(s_instance, &createInfo, nullptr, &m_debugMessenger));
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		createInfo.pfnUserCallback = VulkanDebugCallback;
		createInfo.pUserData = nullptr;
	}

	bool VulkanContext::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availibleLayers{ layerCount };
		vkEnumerateInstanceLayerProperties(&layerCount, availibleLayers.data());

		for (const char* layerName : m_validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availibleLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanContext::GetRequiredExtensions()
	{
		uint32_t extensionCount = 0;
		const char** extensions;

		extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

		std::vector<const char*> extensionsVector{ extensions, extensions + extensionCount };

	#ifdef LP_VALIDATION
		extensionsVector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	#endif
		return extensionsVector;
	}
}