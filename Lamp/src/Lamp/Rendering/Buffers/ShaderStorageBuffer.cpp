#include "lppch.h"
#include "ShaderStorageBuffer.h"

#include "Platform/Vulkan/VulkanShaderStorageBuffer.h"

namespace Lamp
{
	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size)
	{
		return CreateRef<VulkanShaderStorageBuffer>(size);
	}
}