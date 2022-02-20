#pragma once

#include "Lamp/Rendering/RenderPass.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include <yaml-cpp/yaml.h>
#include <string>


namespace Lamp
{
	class RenderNode;
	struct RenderLink;

	enum class RenderNodeType : uint32_t
	{
		Pass = 0,
		Framebuffer = 1,
		Texture = 2,
		Start = 3,
		End = 4,
		Compute = 5
	};

	enum class RenderAttributeType : uint32_t
	{
		Texture = 0,
		Framebuffer = 1,
		Pass = 2,
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
		bool shouldDraw = false;
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

	class RenderNode
	{
	public:
		RenderNode() = default;
		virtual ~RenderNode() = default;

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

		void DrawAttributes(const std::vector<Ref<RenderInputAttribute>>& inputs, const std::vector<Ref<RenderOutputAttribute>>& outputs);
		void SerializeAttributes(YAML::Emitter& out);
		void DeserializeAttributes(YAML::Node& node);

		void SerializeBaseAttribute(Ref<RenderAttribute> attr, const std::string& attrType, YAML::Emitter& out);
		std::pair<Ref<RenderAttribute>, std::string> DeserializeBaseAttribute(const YAML::Node& node);

	protected:
		bool IsAttributeLinked(Ref<RenderAttribute> attr);
		Ref<RenderAttribute> FindAttributeByID(GraphUUID id);
	};
}