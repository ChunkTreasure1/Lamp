
#include "lppch.h"
#include "UIUtility.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include <imgui/backends/imgui_impl_vulkan.h>

using namespace Lamp;


ImTextureID UI::GetTextureID(Ref<Lamp::Texture2D> texture)
{
	Ref<VulkanTexture2D> vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(texture);
	const VkDescriptorImageInfo& imageInfo = vulkanTexture->GetDescriptorInfo();

	if (!imageInfo.imageView)
	{
		return nullptr;
	}

	ImTextureID id = ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);

	return id;
}

ImTextureID UI::GetTextureID(Ref<Lamp::Image2D> image)
{
	Ref<VulkanImage2D> vulkanTexture = std::reinterpret_pointer_cast<VulkanImage2D>(image);
	const VkDescriptorImageInfo& imageInfo = vulkanTexture->GetDescriptorInfo();

	if (!imageInfo.imageView)
	{
		return nullptr;
	}

	ImTextureID id = ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);
	return id;
}