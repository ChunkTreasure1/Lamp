#include "lppch.h"
#include "RenderNodeEnd.h"

#include "RenderNodePass.h"

#include <imnodes.h>

namespace Lamp
{
	void RenderNodeEnd::Initialize()
	{
		Ref<RenderInputAttribute> endAttr = CreateRef<RenderInputAttribute>();
		endAttr->pNode = this;
		endAttr->name = "End";
		endAttr->type = RenderAttributeType::Pass;

		inputs.push_back(endAttr);
	}

	void RenderNodeEnd::Start()
	{}

	void RenderNodeEnd::DrawNode()
	{
		LP_PROFILE_FUNCTION();

		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(36, 166, 151, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(107, 194, 184, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(107, 194, 184, 255));

		ImNodes::BeginNode(id);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("End");
		ImNodes::EndNodeTitleBar();

		DrawAttributes();

		ImNodes::EndNode();

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeEnd::Activate(std::any value)
	{
		LP_PROFILE_FUNCTION();
		framebuffer = std::any_cast<Ref<Framebuffer>>(value);
	}

	void RenderNodeEnd::Serialize(YAML::Emitter& out)
	{
		SerializeAttributes(out);
	}

	void RenderNodeEnd::Deserialize(YAML::Node& node)
	{
		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}
}