#pragma once

#include <any>

namespace Lamp
{
	enum class TextureType : uint32_t
	{
		Color = 0,
		Depth = 1
	};

	enum class FramebufferCommand : uint32_t
	{
		Copy = 0
	};

	struct PassStaticUniformSpecification
	{
		PassStaticUniformSpecification(const std::string& aName = "Uniform", UniformType aType = UniformType::Int, const std::any& aData = 0)
			: name(aName), type(aType), data(std::move(aData))
		{}

		std::string name;
		UniformType type;
		std::any data;
	};

	struct PassDynamicUniformSpecification
	{
		PassDynamicUniformSpecification(const std::string& aName = "Uniform", UniformType aType = UniformType::Int, void* aData = nullptr)
			: name(aName), type(aType), data(aData)
		{}

		std::string name;
		UniformType type;
		void* data;
	};

	struct PassTextureSpecification
	{
		PassTextureSpecification(Ref<Texture2D> aTexture = nullptr, uint32_t aBindSlot = 0)
			: texture(aTexture), bindSlot(aBindSlot)
		{}

		Ref<Texture2D> texture;
		uint32_t bindSlot;
	};

	struct PassFramebufferAttachmentSpec
	{
		bool operator==(const PassFramebufferAttachmentSpec& rhs)
		{
			return type == rhs.type && bindId == rhs.bindId && attachmentId == rhs.attachmentId;
		}

		TextureType type = TextureType::Color;
		uint32_t bindId = 0;
		uint32_t attachmentId = 0;
	};

	struct PassFramebufferSpecification
	{
		PassFramebufferSpecification(Ref<Framebuffer> aFramebuffer = nullptr, std::vector<PassFramebufferAttachmentSpec> aAttachments = std::vector<PassFramebufferAttachmentSpec>())
			: framebuffer(aFramebuffer), attachments(aAttachments)
		{}

		Ref<Framebuffer> framebuffer;
		std::vector<PassFramebufferAttachmentSpec> attachments;
	};

	struct PassFramebufferCommandSpecification
	{
		Ref<Framebuffer> primary;
		Ref<Framebuffer> secondary;

		FramebufferCommand command;
	};
}