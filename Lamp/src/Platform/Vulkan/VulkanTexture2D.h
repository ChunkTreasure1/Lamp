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
		VulkanTexture2D(const std::filesystem::path& path, bool generateMips);

		~VulkanTexture2D();

		void Bind(uint32_t slot /* = 0 */) const override;
		void SetData(const void* data, uint32_t size) override;

		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const uint32_t GetID() const override;

		inline const VkDescriptorImageInfo& GetDescriptorInfo() const { return m_image->GetDescriptorInfo(); }
		inline const Ref<VulkanImage2D> GetImage() const { return m_image; }

	private:
		Ref<VulkanImage2D> m_image;
	};
}