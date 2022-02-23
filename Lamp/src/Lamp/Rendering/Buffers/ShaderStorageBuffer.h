#pragma once

#include "Lamp/Core/Core.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <vulkan/vulkan.h>

namespace Lamp
{
	enum class DrawAccess
	{
		Static,
		Dynamic
	};

	class ShaderStorageBuffer
	{
	public:
	public:
		ShaderStorageBuffer(uint32_t size);
		~ShaderStorageBuffer();

		void SetData(const void* data, uint32_t size);
		void* Map();
		void Unmap();

		inline const VkDescriptorBufferInfo& GetDescriptorInfo() const { return m_descriptorInfo; }

		static Ref<ShaderStorageBuffer> Create(uint32_t size);

	private:
		VkDescriptorBufferInfo m_descriptorInfo;
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
	};
}