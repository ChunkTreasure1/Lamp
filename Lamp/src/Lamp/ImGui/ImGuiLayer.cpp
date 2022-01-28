#include "lppch.h"
#include "ImGuiLayer.h"

#include "Platform/Vulkan/VulkanImGuiLayer.h"

namespace Lamp
{
	ImGuiLayer* ImGuiLayer::Create()
	{
		return new VulkanImGuiLayer();
	}
}