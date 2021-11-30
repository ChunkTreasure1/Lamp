#pragma once

#include "Platform/Vulkan/VulkanContext.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>

//Based on Hazel
namespace Lamp
{
	struct GPUMemoryStatistics
	{
		uint64_t allocated = 0;
		uint64_t free = 0;
	};

	class VulkanDevice;
	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;
		VulkanAllocator(const std::string& tag);
		~VulkanAllocator();

		VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer);
		VmaAllocation AllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage);

		void Free(VmaAllocation allocation);
		void DestroyImage(VkImage image, VmaAllocation allocation);
		void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

		template<typename T>
		T* MapMemory(VmaAllocation allocation)
		{
			T* mappedMemory;
			vmaMapMemory(VulkanAllocator::GetVMAAllocator(), allocation, (void**)&mappedMemory);
			return mappedMemory;
		}

		void UnmapMemory(VmaAllocation allocation);

		static void Initialize(VulkanDevice* device);
		static void Shutdown();
		static VmaAllocator& GetVMAAllocator();
		static GPUMemoryStatistics GetStatistics();

	private:
		std::string m_tag;
	};
}