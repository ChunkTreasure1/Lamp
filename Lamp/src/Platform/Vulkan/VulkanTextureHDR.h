#pragma once

#include "Lamp/Rendering/Textures/TextureHDR.h"
#include "Platform/Vulkan/VulkanImage2D.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class VulkanImage2D;
	class VulkanTextureHDR : public TextureHDR
	{
	public:
		VulkanTextureHDR(uint32_t width, uint32_t height);
		VulkanTextureHDR(const std::filesystem::path& path);
		~VulkanTextureHDR();

		void Bind(uint32_t slot /* = 0 */) const override;
		void SetData(const void* data, uint32_t size) override;

		const uint32_t GetWidth() const override { return m_image->GetWidth(); }
		const uint32_t GetHeight() const override { return m_image->GetHeight(); }
		const uint32_t GetID() const override { return 0; }

		inline const VkDescriptorImageInfo& GetDescriptorInfo() const { return m_image->GetDescriptorInfo(); }
		inline const Ref<VulkanImage2D> GetImage() const { return m_image; }

	private:
		Ref<VulkanImage2D> m_image;
	};
}