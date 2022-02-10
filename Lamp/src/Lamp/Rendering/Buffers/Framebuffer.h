#pragma once

#include "Lamp/Rendering/Textures/Image2D.h"

namespace Lamp
{
	enum class FramebufferRenderbufferType : uint32_t
	{
		Color = 0,
		Depth
	};

	enum class ClearMode : uint32_t
	{
		Clear,
		Load,
		DontCare
	};



	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() 
			: textureFiltering(TextureFilter::Linear), textureWrap(TextureWrap::Repeat), textureFormat(ImageFormat::RGBA)
		{}
		FramebufferTextureSpecification(
			ImageFormat format,
			TextureFilter filtering = TextureFilter::Linear,
			TextureWrap wrap = TextureWrap::Repeat,
			TextureBlend blend = TextureBlend::None,
			const glm::vec4& borderColor = { 1.f, 1.f, 1.f, 1.f },
			bool sampled = false)
			: textureFormat(format), textureFiltering(filtering), textureWrap(wrap), BorderColor(borderColor)
		{}

		bool operator==(const FramebufferTextureSpecification& second)
		{
			return this == &second;
		}

		ImageFormat textureFormat;
		TextureFilter textureFiltering;
		TextureWrap textureWrap;
		TextureBlend blending;
		ClearMode clearMode = ClearMode::Clear;

		bool MultiSampled = false;
		
		glm::vec4 BorderColor = { 1.f, 1.f, 1.f, 1.f };
		std::string name = "Attachment";
	};

	struct FramebufferRenderbufferSpecification
	{
		FramebufferRenderbufferSpecification() = default;
		FramebufferRenderbufferSpecification(FramebufferRenderbufferType format)
			: Format(format)
		{}

		FramebufferRenderbufferType Format;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments(attachments)
		{}

		std::vector<FramebufferTextureSpecification> Attachments;
		std::vector<FramebufferRenderbufferSpecification> Renderbuffers;
	};

	struct FramebufferSpecification
	{
		uint32_t width = 1280;
		uint32_t height = 720;

		glm::vec4 clearColor = { 1.f, 1.f, 1.f, 1.f };
		uint32_t samples = 1;
		FramebufferAttachmentSpecification attachments;

		bool swapchainTarget = false;
		bool copyable = false;
		bool shadow = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Resize(const uint32_t width, const uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;
		virtual void Copy(uint32_t rendererId, const glm::vec2& size, bool depth = false) = 0;
		virtual void Invalidate() = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		virtual Ref<Image2D> GetColorAttachment(uint32_t index) const = 0;
		virtual Ref<Image2D> GetDepthAttachment() const = 0;

		//TODO: deprecate
		virtual inline const uint32_t GetColorAttachmentID(uint32_t i = 0) = 0;
		virtual inline const uint32_t GetDepthAttachmentID() = 0;
		virtual inline const uint32_t GetRendererID() = 0;

		virtual void BindColorAttachment(uint32_t id = 0, uint32_t i = 0) = 0;
		virtual void BindDepthAttachment(uint32_t id = 0) = 0;

		virtual FramebufferSpecification& GetSpecification() = 0;

	public:
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}