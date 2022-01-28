
#include "lppch.h"
#include "UIUtility.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include <imgui/backends/imgui_impl_vulkan.h>

using namespace Lamp;

static std::unordered_map<Ref<Lamp::Texture2D>, ImTextureID> s_textureIdCache;
static std::unordered_map<Ref<Lamp::Image2D>, ImTextureID> s_imageIdCache;

ImTextureID UI::GetTextureID(Ref<Lamp::Texture2D> texture)
{
	auto it = s_textureIdCache.find(texture);
	if (it != s_textureIdCache.end())
	{
		return it->second;
	}

	Ref<VulkanTexture2D> vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(texture);
	const VkDescriptorImageInfo& imageInfo = vulkanTexture->GetDescriptorInfo();

	if (!imageInfo.imageView)
	{
		return nullptr;
	}

	ImTextureID id = ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);
	s_textureIdCache[texture] = id;

	return id;
}

ImTextureID UI::GetTextureID(Ref<Lamp::Image2D> texture)
{
	Ref<VulkanImage2D> vulkanTexture = std::reinterpret_pointer_cast<VulkanImage2D>(texture);
	const VkDescriptorImageInfo& imageInfo = vulkanTexture->GetDescriptorInfo();

	if (!imageInfo.imageView)
	{
		return nullptr;
	}

	ImTextureID id = ImGui_ImplVulkan_AddTexture(imageInfo.sampler, imageInfo.imageView, imageInfo.imageLayout);

	auto it = s_imageIdCache.find(texture);
	if (it != s_imageIdCache.end())
	{
		auto device = VulkanContext::GetCurrentDevice();

		VkDescriptorSet descriptor = (VkDescriptorSet)it->second;
		vkFreeDescriptorSets(device->GetHandle(), std::reinterpret_pointer_cast<VulkanRenderer>(Renderer::GetRenderer())->GetDescriptorPool(), 1, &descriptor);
	}

	s_imageIdCache[texture] = id;

	return id;
}
