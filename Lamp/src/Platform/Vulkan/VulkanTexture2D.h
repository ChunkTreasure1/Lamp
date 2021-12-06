#pragma once

#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Platform/Vulkan/VulkanImage2D.h"

#include <vulkan/vulkan_core.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(uint32_t width, uint32_t height);
		VulkanTexture2D(const std::filesystem::path& path);

		~VulkanTexture2D();

		virtual void Bind(uint32_t slot /* = 0 */) const override;
		virtual void SetData(const void* data, uint32_t size) override;

		virtual const uint32_t GetWidth() const override;
		virtual const uint32_t GetHeight() const override;
		virtual const uint32_t GetID() const override;

		inline VkDescriptorImageInfo GetDescriptorInfo() const { return m_image->GetDescriptorInfo(); }

	private:
		Ref<VulkanImage2D> m_image;
	};
}