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

	struct PassUniformSpecification
	{
		PassUniformSpecification(const std::string& aName = "Uniform", UniformType aType = UniformType::Int, const std::any& aData = 0, uint32_t aId = 0)
			: name(aName), type(aType), data(aData), id(aId)
		{}

		PassUniformSpecification(void* aData, const std::string& aName = "Uniform", UniformType aType = UniformType::Int, uint32_t aId = 0)
			: name(aName), type(aType), pData(aData), id(aId)
		{ }

		std::string name;
		UniformType type;
		std::any data;
		void* pData = nullptr;
		uint32_t id;
	};

	struct PassTextureSpecification
	{
		PassTextureSpecification(Ref<Texture2D> aTexture = nullptr, uint32_t aBindSlot = 0, const std::string& aName = "Texture")
			: texture(aTexture), bindSlot(aBindSlot), name(aName)
		{}

		Ref<Texture2D> texture;
		uint32_t bindSlot;
		std::string name;
	};

	struct PassFramebufferAttachmentSpec
	{
		bool operator==(const PassFramebufferAttachmentSpec& rhs)
		{
			return this == &rhs;
		}

		TextureType type = TextureType::Color;
		uint32_t bindId = 0;
		uint32_t attachmentId = 0;
	};

	struct PassFramebufferSpecification
	{
		PassFramebufferSpecification(Ref<Framebuffer> aFramebuffer = nullptr, std::vector<PassFramebufferAttachmentSpec> aAttachments = std::vector<PassFramebufferAttachmentSpec>(), const std::string& aName = "Framebuffer")
			: framebuffer(aFramebuffer), attachments(aAttachments), name(aName)
		{}

		Ref<Framebuffer> framebuffer;
		std::vector<PassFramebufferAttachmentSpec> attachments;
		std::string name;
	};

	struct PassFramebufferCommandSpecification
	{
		PassFramebufferCommandSpecification(Ref<Framebuffer> aPrimary = nullptr, Ref<Framebuffer> aSecondary = nullptr, FramebufferCommand aCommand = FramebufferCommand::Copy, const std::string& aName = "Command")
			: primary(aPrimary), secondary(aSecondary), command(aCommand), name(aName)
		{}

		Ref<Framebuffer> primary;
		Ref<Framebuffer> secondary;

		FramebufferCommand command;
		std::string name;
	};
}