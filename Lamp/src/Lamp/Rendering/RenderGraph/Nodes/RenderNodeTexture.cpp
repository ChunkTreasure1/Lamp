#include "lppch.h"
#include "RenderNodeTexture.h"

#include "RenderNodePass.h"
#include "Lamp/AssetSystem/ResourceCache.h"
#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/Utility/SerializeMacros.h"

#include <imgui.h>
#include <imnodes.h>
#include <imgui_stdlib.h>

namespace Lamp
{
	void RenderNodeTexture::Initialize()
	{
		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->pNode = this;
		output->name = "Output";
		output->id = ++currId;
		output->type = RenderAttributeType::Texture;

		outputs.push_back(output);
	}

	void RenderNodeTexture::Start()
	{
		for (auto& link : links)
		{
			if (RenderNodePass* passNode = dynamic_cast<RenderNodePass*>(link->pInput->pNode))
			{
				uint32_t index = std::any_cast<uint32_t>(link->pInput->data);
				auto& [passTex, bindId] = passNode->renderPass->GetSpecification().textures[index];
				passTex = texture;
			}
		}
	}

	void RenderNodeTexture::DrawNode()
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(62, 189, 100, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(100, 181, 124, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(100, 181, 124, 255));

		ImNodes::BeginNode(id);

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("Texture node");
		ImNodes::EndNodeTitleBar();

		ImGui::Text("Texture:");
		if (texture)
		{
			if (ImGui::ImageButton((ImTextureID)texture->GetID(), { 100, 100 }, { 0, 1 }, { 1, 0 }))
			{
				GetTexture();
			}
		}
		else
		{
			if (ImGui::Button("Choose a texture!", { 128, 128 }))
			{
				GetTexture();
			}
		}

		for (auto& output : outputs)
		{
			ImNodes::BeginOutputAttribute(output->id);
			ImGui::Text("Output");
			ImNodes::EndOutputAttribute();
		}

		ImNodes::EndNode();

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeTexture::Serialize(YAML::Emitter& out)
	{
		//TODO: change to use asset handle
		LP_SERIALIZE_PROPERTY(path, (texture ? texture->Path.string() : ""), out);

		uint32_t attrId = 0;
		for (auto &input : inputs)
		{
			SerializeBaseAttribute(input, "input", out, attrId);
			attrId++;
		}

		for (auto &output : outputs)
		{
			SerializeBaseAttribute(output, "output", out, attrId);
			attrId++;
		}
	}

	void RenderNodeTexture::Deserialize(YAML::Node& node)
	{
		std::string texPath = node["path"].as<std::string>();
		if (!texPath.empty())
		{
			texture = ResourceCache::GetAsset<Texture2D>(texPath);
		}

		//attributes
		outputs.clear();
		inputs.clear();
		uint32_t attributeCount = 0;

		while (YAML::Node attribute = node["attribute" + std::to_string(attributeCount)])
		{
			const auto& [attr, attrType] = DeserializeBaseAttribute(attribute);
			attr->pNode = this;
			if (attrType == "input")
			{
				inputs.push_back(std::dynamic_pointer_cast<RenderInputAttribute>(attr));
			}
			else
			{
				outputs.push_back(std::dynamic_pointer_cast<RenderOutputAttribute>(attr));
			}

			currId = attr->id;
			attributeCount++;
		}
	}

	void RenderNodeTexture::GetTexture()
	{
		std::string path = FileDialogs::OpenFile("All (*.*)\0*.*\0");
		if (!path.empty())
		{
			Ref<Texture2D> tex = ResourceCache::GetAsset<Texture2D>(path);
			if (tex->IsValid())
			{
				texture = tex;
			}
		}
	}
}