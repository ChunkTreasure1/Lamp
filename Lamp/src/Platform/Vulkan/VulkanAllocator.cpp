#include "lppch.h"
#include "VulkanAllocator.h"

#include "Lamp/Core/Log.h"
#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanDevice.h"

namespace Lamp
{
	struct VulkanAllocatorData
	{
		VmaAllocator allocator;
		uint64_t totalFreedBytes = 0;
		uint64_t totalAllocatedBytes = 0;
	};

	static VulkanAllocatorData* s_pData = nullptr;
	static GPUMemoryStatistics s_memoryStatistics;

	void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
	{
		vmaUnmapMemory(s_pData->allocator, allocation);
	}

	void VulkanAllocator::Initialize(VulkanDevice* device)
	{
		s_pData = new VulkanAllocatorData();

		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = device->GetPhysicalDevice()->GetHandle();
		allocatorInfo.device = device->GetHandle();
		allocatorInfo.instance = VulkanContext::GetVulkanInstance();
		allocatorInfo.pRecordSettings;

		VkResult result = vmaCreateAllocator(&allocatorInfo, &s_pData->allocator);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create allocator!");
	}

	void VulkanAllocator::Shutdown()
	{
		vmaDestroyAllocator(s_pData->allocator);

		delete s_pData;
		s_pData = nullptr;
	}

	VmaAllocator& VulkanAllocator::GetVMAAllocator()
	{
		return s_pData->allocator;
	}

	const GPUMemoryStatistics& VulkanAllocator::GetStatistics()
	{
		const auto& memProps = VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetMemoryProperties();
		s_memoryStatistics.totalGPUMemory = memProps.memoryHeaps[0].size;
		s_memoryStatistics.allocatedMemory = s_pData->totalAllocatedBytes - s_pData->totalFreedBytes;
		s_memoryStatistics.freeMemory = s_memoryStatistics.totalGPUMemory - s_memoryStatistics.allocatedMemory;

		return s_memoryStatistics;
	}

	VulkanAllocator::VulkanAllocator(const std::string& tag)
		: m_tag(tag)
	{
	}

	VulkanAllocator::~VulkanAllocator()
	{
	}

	VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer& outBuffer)
	{
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateBuffer(s_pData->allocator, &bufferCreateInfo, &allocCreateInfo, &outBuffer, &allocation, nullptr);

		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_pData->allocator, allocation, &allocInfo);

		s_pData->totalAllocatedBytes += allocInfo.size;
		LP_CORE_INFO("VulkanAllocator: Total allocation: " + std::to_string(s_pData->totalAllocatedBytes));
		LP_CORE_INFO("VulkanAllocator: Currently allocated: {0}", (s_pData->totalAllocatedBytes - s_pData->totalFreedBytes));

		return allocation;
	}

	VmaAllocation VulkanAllocator::AllocateImage(VkImageCreateInfo imageCreateInfo, VmaMemoryUsage usage, VkImage& outImage)
	{
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = usage;

		VmaAllocation allocation;
		vmaCreateImage(s_pData->allocator, &imageCreateInfo, &allocCreateInfo, &outImage, &allocation, nullptr);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(s_pData->allocator, allocation, &allocInfo);

		s_pData->totalAllocatedBytes += allocInfo.size;
		LP_CORE_INFO("VulkanAllocator: Total allocation: " + std::to_string(s_pData->totalAllocatedBytes));
		LP_CORE_INFO("VulkanAllocator: Currently allocated: {0}", (s_pData->totalAllocatedBytes - s_pData->totalFreedBytes));

		return allocation;
	}

	void VulkanAllocator::Free(VmaAllocation allocation)
	{
		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_pData->allocator, allocation, &allocInfo);
		s_pData->totalFreedBytes += allocInfo.size;
		LP_CORE_INFO("Total freed: " + std::to_string(s_pData->totalFreedBytes));

		vmaFreeMemory(s_pData->allocator, allocation);
	}

	void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation)
	{
		LP_CORE_ASSERT(image, "Image is nullptr!");
		LP_CORE_ASSERT(allocation, "Allocation is nullptr!");

		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_pData->allocator, allocation, &allocInfo);
		s_pData->totalFreedBytes += allocInfo.size;
		LP_CORE_INFO("VulkanAllocator: Total freed: " + std::to_string(s_pData->totalFreedBytes));
		LP_CORE_INFO("VulkanAllocator: Currently allocated: {0}", (s_pData->totalAllocatedBytes - s_pData->totalFreedBytes));

		vmaDestroyImage(s_pData->allocator, image, allocation);
	}

	void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
	{
		LP_CORE_ASSERT(buffer, "Buffer is nullptr!");
		LP_CORE_ASSERT(allocation, "Allocation is nullptr!");

		VmaAllocationInfo allocInfo{};
		vmaGetAllocationInfo(s_pData->allocator, allocation, &allocInfo);
		s_pData->totalFreedBytes += allocInfo.size;
		LP_CORE_INFO("VulkanAllocator: Total freed: " + std::to_string(s_pData->totalFreedBytes));
		LP_CORE_INFO("VulkanAllocator: Currently allocated: {0}", (s_pData->totalAllocatedBytes - s_pData->totalFreedBytes));

		vmaDestroyBuffer(s_pData->allocator, buffer, allocation);
	}
}