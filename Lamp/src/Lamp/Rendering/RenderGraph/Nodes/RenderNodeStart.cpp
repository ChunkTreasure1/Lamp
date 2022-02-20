#include "lppch.h"
#include "RenderNodeStart.h"

#include "Lamp/Rendering/Cameras/CameraBase.h"
#include "Lamp/Rendering/RenderCommand.h"

#include <imnodes.h>

namespace Lamp
{
	void RenderNodeStart::Initialize()
	{
		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->pNode = this;
		output->name = "Start";
		output->type = RenderAttributeType::Pass;

		outputs.emplace_back(output);
	}

	void RenderNodeStart::Start()
	{
	}

	void RenderNodeStart::DrawNode()
	{
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
		ImGui::Text("Start");
		ImNodes::EndNodeTitleBar();

		DrawAttributes(inputs, outputs);

		ImNodes::EndNode();

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeStart::Activate(std::any value)
	{
		RenderCommand::Begin(std::any_cast<Ref<CameraBase>>(value));

		for (const auto& link : links)
		{
			link->pInput->pNode->Activate(value);
		}

		RenderCommand::End();
	}

	RenderNodeType RenderNodeStart::GetNodeType()
	{
		return RenderNodeType::Start;
	}

	void RenderNodeStart::Serialize(YAML::Emitter& out)
	{
		SerializeAttributes(out);
	}

	void RenderNodeStart::Deserialize(YAML::Node& node)
	{
		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}
}