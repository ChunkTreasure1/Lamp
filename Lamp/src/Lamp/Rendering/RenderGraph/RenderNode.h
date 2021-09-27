#pragma once

#include <string>

#include "Lamp/Rendering/RenderPass.h"
#include <yaml-cpp/yaml.h>

namespace Lamp
{
	struct RenderNode;
	struct RenderLink;

	enum class RenderNodeType : uint32_t
	{
		Pass = 0,
		Framebuffer = 1,
		Texture = 2,
		DynamicUniform = 3
	};

	enum class RenderAttributeType : uint32_t
	{
		Texture = 0,
		Framebuffer = 1,
		DynamicUniform = 2
	};

	struct RenderAttribute
	{
		virtual ~RenderAttribute() {}

		std::vector<Ref<RenderLink>> links;
		RenderNode* pNode;
		uint32_t id;
		std::string name;
		RenderAttributeType type;
	};

	struct RenderOutputAttribute : public RenderAttribute
	{};

	struct RenderInputAttribute : public RenderAttribute
	{
		std::any data;
		void* pData = nullptr;
	};

	struct RenderLink
	{
		RenderOutputAttribute* pOutput;
		RenderInputAttribute* pInput;
		uint32_t id;
	};

	struct RenderNode
	{
		RenderNode()
			: currId(UINT32_MAX - 1000)
		{}
		virtual ~RenderNode() {}

		glm::vec2 position = { 0.f, 0.f };

		std::vector<Ref<RenderLink>> links;
		std::vector<Ref<RenderOutputAttribute>> outputs;
		std::vector<Ref<RenderInputAttribute>> inputs;

		uint32_t currId;
		uint32_t id;

		virtual void Initialize() = 0;
		virtual void Start() = 0;
		virtual void DrawNode() = 0;
		virtual void Activate(std::any value) = 0;
		virtual RenderNodeType GetNodeType() = 0;
		virtual void Serialize(YAML::Emitter& out) = 0;
		virtual void Deserialize(YAML::Node& node) = 0;

		void SerializeBaseAttribute(Ref<RenderAttribute> attr, const std::string& attrType, YAML::Emitter& out, uint32_t id);
		std::pair<Ref<RenderAttribute>, std::string> DeserializeBaseAttribute(const YAML::Node& node);
	};
}