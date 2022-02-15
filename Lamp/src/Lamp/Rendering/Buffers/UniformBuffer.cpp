#include "lppch.h"
#include "UniformBuffer.h"

#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace Lamp
{
	Ref<UniformBuffer> UniformBuffer::Create(const void* data, uint32_t size)
	{
		return CreateRef<VulkanUniformBuffer>(data, size);
	}
}

