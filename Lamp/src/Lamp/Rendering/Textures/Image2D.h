#pragma once

#include <vulkan/vulkan.h>
#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	enum class ImageFormat
	{
		None = 0,
		R32F,
		R32SI,
		R32UI,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,
		RG16F,
		RG32F,

		SRGB,

		DEPTH32F,
		DEPTH24STENCIL8
	};

	enum class AniostopyLevel : uint32_t
	{
		X2 = 2,
		X4 = 4,
		X8 = 8,
		X16 = 16
	};

	enum class ImageUsage
	{
		None = 0,
		Texture,
		Attachment,
		Storage
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp,
		Repeat
	};

	enum class TextureFilter
	{
		None = 0,
		Linear,
		Nearest
	};

	enum class TextureBlend : uint32_t
	{
		None,
		Min,
		Max
	};

	struct ImageSpecification
	{
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t mips = 1;
		uint32_t layers = 1;

		ImageFormat format = ImageFormat::RGBA;
		ImageUsage usage = ImageUsage::Texture;
		TextureWrap wrap = TextureWrap::Repeat;
		TextureFilter filter = TextureFilter::Linear;

		AniostopyLevel level = AniostopyLevel::X4;

		std::string debugName;
		bool useAniostopy = false;
		bool copyable = false;
		bool comparable = false;
	};

	class CommandBuffer;

	class Image2D
	{
	public:
		Image2D(const ImageSpecification& specification, const void* data);
		~Image2D();

		void Invalidate(const void* data);
		void Release();

		void TransitionToLayout(Ref<CommandBuffer> commandBuffer, VkImageLayout layout);

		uint32_t GetWidth() const { return m_specification.width; }
		uint32_t GetHeight() const { return m_specification.height; }
		float GetAspectRatio() const { return (float)m_specification.width / (float)m_specification.height; }

		inline const VkDescriptorImageInfo& GetDescriptorInfo() const { return m_descriptorInfo; }
		inline VkImage GetHandle() const { return m_image; }
		inline VkImageView GetImageView() const { return m_imageViews.at(0); }
		inline VkFormat GetFormat() const { return m_format; }

		const ImageSpecification& GetSpecification() { return m_specification; }
		
		static Ref<Image2D> Create(const ImageSpecification& specification, const void* data = nullptr);

	private:
		void UpdateDescriptor();

		ImageSpecification m_specification;

		VmaAllocation m_allocation = nullptr;

		VkDescriptorImageInfo m_descriptorInfo;
		VkImage m_image = nullptr;
		VkSampler m_sampler = nullptr;
		VkFormat m_format;
		VkImageLayout m_imageLayout;

		std::map<uint32_t, VkImageView> m_imageViews;

	};

	namespace Utils
	{
		inline bool IsDepthFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::DEPTH32F: return true;
				case ImageFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}
	}
}