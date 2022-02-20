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
		VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height);
		VulkanTexture2D(const std::filesystem::path& path, bool generateMips);
		VulkanTexture2D() = default;

		~VulkanTexture2D() override;

		void Bind(uint32_t slot /* = 0 */) const override;
		void SetData(const void* data, uint32_t size) override;

		const uint32_t GetWidth() const override;
		const uint32_t GetHeight() const override;
		const uint32_t GetID() const override;

		void Load(const std::filesystem::path& path, bool generateMips /* = true */);

		const VkDescriptorImageInfo& GetDescriptorInfo() const;
		inline const Ref<VulkanImage2D> GetImage() const { return m_image; }

	private:
		void LoadKTX(const std::filesystem::path& path, bool generateMips);
		void LoadOther(const std::filesystem::path& path, bool generateMips);
		void Setup(void* data, uint32_t size, uint32_t width, uint32_t height, bool generateMips, bool isHDR);

		Ref<VulkanImage2D> m_image;
	};
}