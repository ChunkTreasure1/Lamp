
#include "lppch.h"
#include "UIUtility.h"

#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Textures/Image2D.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include <imgui/backends/imgui_impl_vulkan.h>

using namespace Lamp;


ImTextureID UI::GetTextureID(Ref<Lamp::Texture2D> texture)
{
	const VkDescriptorImageInfo& imageInfo = texture->GetDescriptorInfo();

	if (!imageInfo.imageView)
	{
		return nullptr;
	}

	ImTextureID id = ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);

	return id;
}

ImTextureID UI::GetTextureID(Lamp::Texture2D* texture)
{
	const VkDescriptorImageInfo& imageInfo = texture->GetDescriptorInfo();

	if (!imageInfo.imageView)
	{
		return nullptr;
	}

	ImTextureID id = ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);

	return id;
}

ImTextureID UI::GetTextureID(Ref<Lamp::Image2D> image)
{
	const VkDescriptorImageInfo& imageInfo = image->GetDescriptorInfo();

	if (!imageInfo.imageView)
	{
		return nullptr;
	}

	ImTextureID id = ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);
	return id;
}