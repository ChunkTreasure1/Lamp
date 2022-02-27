#pragma once

#include "Lamp/Rendering/Textures/Image2D.h"

#include <vulkan/vulkan.h>

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

		std::map<uint32_t, Ref<Image2D>> existingImages;

		bool swapchainTarget = false;
		bool copyable = false;
		bool shadow = false;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& spec);
		~Framebuffer();

		void Bind(Ref<CommandBuffer> commandBuffer);
		void Unbind(Ref<CommandBuffer> commandBuffer);

		void Resize(const uint32_t width, const uint32_t height);
		int ReadPixel(uint32_t attachmentIndex, int x, int y);
		void Copy(uint32_t rendererId, const glm::vec2& size, bool depth /* = false */);
		void Invalidate();

		void ClearAttachment(uint32_t attachmentIndex, int value);

		const VkRenderingAttachmentInfo& GetColorAttachmentInfo(uint32_t index) const;
		const VkRenderingAttachmentInfo& GetDepthAttachmentInfo() const;
		const std::vector<VkRenderingAttachmentInfo>& GetColorAttachmentInfos() const;

		const std::vector<VkFormat>& GetColorFormats();
		const VkFormat& GetDepthFormat();

		Ref<Image2D> GetColorAttachment(uint32_t index) const;
		Ref<Image2D> GetDepthAttachment() const;

		FramebufferSpecification& GetSpecification();

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	private:
		FramebufferSpecification m_specification;

		uint32_t m_width;
		uint32_t m_height;

		Ref<Image2D> m_depthAttachmentImage;
		std::vector<Ref<Image2D>> m_attachmentImages;

		std::vector<VkFormat> m_colorFormats;
		VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;

		std::vector<VkRenderingAttachmentInfo> m_colorAttachmentInfos;
		VkRenderingAttachmentInfo m_depthAttachmentInfo;
	};
}