#pragma once

namespace Lamp
{
	enum class FramebufferTextureFormat
	{
		None = 0,
		RGBA8 = 1,
		RGBA16F = 2,
		RGBA32F = 3,
		RG32F = 4,
		DEPTH32F = 5,
		DEPTH24STENCIL8 = 6,
		Depth = DEPTH24STENCIL8
	};

	enum class FramebufferTexureFiltering
	{
		Nearest = 0,
		Linear = 1,
		NearestMipMapNearest = 2,
		LinearMipMapNearest = 3,
		NearestMipMapLinear = 4,
		LinearMipMapLinear = 5
	};

	enum class FramebufferTextureWrap
	{
		Repeat = 0,
		MirroredRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3,
		MirrorClampToEdge = 4
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format, FramebufferTexureFiltering filtering, FramebufferTextureWrap wrap, const glm::vec4& borderColor = { 1.f, 1.f, 1.f, 1.f })
			: TextureFormat(format), TextureFiltering(filtering), TextureWrap(wrap), BorderColor(borderColor)
		{}

		FramebufferTextureFormat TextureFormat;
		FramebufferTexureFiltering TextureFiltering;
		FramebufferTextureWrap TextureWrap;

		glm::vec4 BorderColor = { 1.f, 1.f, 1.f, 1.f };
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments(attachments)
		{}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;

		glm::vec4 ClearColor;
		uint32_t Samples;
		FramebufferAttachmentSpecification Attachments;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Update(const uint32_t width, const uint32_t height) = 0;

		virtual inline const uint32_t GetColorAttachment() const = 0;
		virtual inline const uint32_t GetDepthAttachment() const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

	public:
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}