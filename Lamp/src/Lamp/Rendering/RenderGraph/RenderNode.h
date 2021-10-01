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
		DynamicUniform = 3,
		Start = 4,
		RenderData = 5,
		End
	};

	enum class RenderAttributeType : uint32_t
	{
		Texture = 0,
		Framebuffer = 1,
		DynamicUniform = 2,
		Pass = 3
	};

	struct RenderAttribute
	{
		RenderAttribute()
			: pNode(nullptr), type(RenderAttributeType::Pass)
		{}
		virtual ~RenderAttribute() {}

		RenderNode* pNode;
		GraphUUID id;
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
		GraphUUID id;

		bool markedForDelete = false;
	};

	struct RenderNode
	{
		RenderNode()
		{}
		virtual ~RenderNode() {}


		std::vector<Ref<RenderLink>> links;
		std::vector<Ref<RenderOutputAttribute>> outputs;
		std::vector<Ref<RenderInputAttribute>> inputs;

		GraphUUID id;
		glm::vec2 position = { 0.f, 0.f };

		virtual void Initialize() = 0;
		virtual void Start() = 0;
		virtual void DrawNode() = 0;
		virtual void Activate(std::any value) = 0;
		virtual RenderNodeType GetNodeType() = 0;
		virtual void Serialize(YAML::Emitter& out) = 0;
		virtual void Deserialize(YAML::Node& node) = 0;

		void DrawAttributes();
		void SerializeAttributes(YAML::Emitter& out);
		void DeserializeAttributes(YAML::Node& node);

		void SerializeBaseAttribute(Ref<RenderAttribute> attr, const std::string& attrType, YAML::Emitter& out);
		std::pair<Ref<RenderAttribute>, std::string> DeserializeBaseAttribute(const YAML::Node& node);
	};
}