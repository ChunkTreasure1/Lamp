#include "lppch.h"
#include "VertexArray.h"

#include "Platform/Vulkan/VulkanVertexArray.h"

namespace Lamp
{
	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<VulkanVertexArray>();
	}
}