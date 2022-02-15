#pragma once

#include "Platform/Vulkan/VulkanContext.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>

//Based on Hazel
namespace Lamp
{
	class VulkanDevice;
	struct GPUMemoryStatistics;

	struct GPUMemoryStatistics
	{
		uint64_t allocatedMemory = 0;
		uint64_t freeMemory = 0;
		uint64_t totalGPUMemory = 0;

		uint64_t totalAllocatedMemory = 0;
		uint64_t totalFreedMemory = 0;
	};

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
		static const GPUMemoryStatistics& GetStatistics();

	private:
		std::string m_tag;
	};
}