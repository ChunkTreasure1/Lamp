#pragma once

namespace Lamp
{
	enum class FramebufferTextureFormat : uint32_t
	{
		None = 0,

		//Color
		RGBA8 = 1,
		RGBA16F = 2,
		RGBA32F = 3,
		RG32F = 4,
		RED_INTEGER = 5,
		RED = 6,

		//Depth
		DEPTH32F = 7,
		DEPTH24STENCIL8 = 8,
		Depth = DEPTH24STENCIL8
	};

	enum class FramebufferRenderbufferType : uint32_t
	{
		Color = 0,
		Depth
	};

	enum class FramebufferTexureFiltering : uint32_t
	{
		Nearest = 0,
		Linear = 1,
		NearestMipMapNearest = 2,
		LinearMipMapNearest = 3,
		NearestMipMapLinear = 4,
		LinearMipMapLinear = 5
	};

	enum class FramebufferTextureWrap : uint32_t
	{
		Repeat = 0,
		MirroredRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3,
		MirrorClampToEdge = 4
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() 
			: TextureFiltering(FramebufferTexureFiltering::Linear), TextureWrap(FramebufferTextureWrap::Repeat)
		{}
		FramebufferTextureSpecification(
			FramebufferTextureFormat format,
			FramebufferTexureFiltering filtering = FramebufferTexureFiltering::Linear,
			FramebufferTextureWrap wrap = FramebufferTextureWrap::Repeat,
			const glm::vec4& borderColor = { 1.f, 1.f, 1.f, 1.f },
			bool sampled = false)
			: TextureFormat(format), TextureFiltering(filtering), TextureWrap(wrap), BorderColor(borderColor)
		{}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		FramebufferTexureFiltering TextureFiltering;
		FramebufferTextureWrap TextureWrap;

		bool MultiSampled = false;
		glm::vec4 BorderColor = { 1.f, 1.f, 1.f, 1.f };
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
		uint32_t Width = 1280;
		uint32_t Height = 720;

		glm::vec4 ClearColor = { 1.f, 1.f, 1.f, 1.f };
		uint32_t Samples = 1;
		FramebufferAttachmentSpecification Attachments;
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