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

	struct PassUniformSpecification
	{
		PassUniformSpecification(const std::string& aName = "Uniform", UniformType aType = UniformType::Int, std::any aData = 0, uint32_t aUniformId = 0, GraphUUID aId = 0, GraphUUID aAttrId = 0)
			: name(aName), type(aType), data(aData), uniformId(aUniformId), id(aId), attributeId(aAttrId)
		{}

		PassUniformSpecification(void* aData, const std::string& aName = "Uniform", UniformType aType = UniformType::Int)
			: name(aName), type(aType), pData(aData)
		{ }

		std::string name;
		UniformType type;
		std::any data = 0;
		void* pData = nullptr;
		uint32_t uniformId = 0;
		
		GraphUUID id = 0;
		GraphUUID attributeId = 0;
	};

	struct PassTextureSpecification
	{
		PassTextureSpecification(Ref<Texture2D> aTexture = nullptr, uint32_t aBindSlot = 0, const std::string& aName = "Texture", GraphUUID aId = 0, GraphUUID attributeId = 0)
			: texture(aTexture), bindSlot(aBindSlot), name(aName), id(aId), attributeId(attributeId)
		{}

		bool operator==(const PassTextureSpecification& rhs) { return this == &rhs; }

		Ref<Texture2D> texture;
		uint32_t bindSlot;
		std::string name;

		GraphUUID id;
		GraphUUID attributeId;
	};

	struct PassFramebufferAttachmentSpec
	{
		bool operator==(const PassFramebufferAttachmentSpec& rhs) { return this == &rhs; }

		TextureType type = TextureType::Color;
		uint32_t bindId = 0;
		uint32_t attachmentId = 0;
	};

	struct PassFramebufferSpecification
	{
		PassFramebufferSpecification(Ref<Framebuffer> aFramebuffer = nullptr, std::vector<PassFramebufferAttachmentSpec> aAttachments = std::vector<PassFramebufferAttachmentSpec>(), const std::string& aName = "Framebuffer", GraphUUID aId = 0, GraphUUID aAttrId = 0)
			: framebuffer(aFramebuffer), attachments(aAttachments), name(aName), id(aId), attributeId(aAttrId)
		{}

		bool operator==(const PassFramebufferSpecification& rhs) { return this == &rhs; }

		Ref<Framebuffer> framebuffer;
		std::vector<PassFramebufferAttachmentSpec> attachments;
		std::string name;

		GraphUUID id;
		GraphUUID attributeId;
	};

	struct PassFramebufferCommandSpecification
	{
		PassFramebufferCommandSpecification(Ref<Framebuffer> aPrimary = nullptr, Ref<Framebuffer> aSecondary = nullptr, FramebufferCommand aCommand = FramebufferCommand::Copy, const std::string& aName = "Command", GraphUUID aId = 0, GraphUUID aAttrId = 0)
			: primary(aPrimary), secondary(aSecondary), command(aCommand), name(aName), id(aId), attributeId(aAttrId)
		{}

		Ref<Framebuffer> primary;
		Ref<Framebuffer> secondary;

		FramebufferCommand command;

		GraphUUID id;
		GraphUUID attributeId;
		std::string name;
	};
}