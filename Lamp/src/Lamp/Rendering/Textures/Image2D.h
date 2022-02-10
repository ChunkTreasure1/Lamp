#pragma once

namespace Lamp
{
	enum class ImageFormat
	{
		None = 0,
		R32F,
		R32I,
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

	class Image2D
	{
	public:
		virtual ~Image2D() = default;
		virtual void Invalidate(const void* data) = 0;
		virtual void Release() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual float GetAspectRatio() const = 0;

		virtual const ImageSpecification& GetSpecification() = 0;

		static Ref<Image2D> Create(const ImageSpecification& specification, const void* data = nullptr);
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