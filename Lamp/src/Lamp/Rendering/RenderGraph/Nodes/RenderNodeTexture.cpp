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
		output->type = RenderAttributeType::Texture;

		outputs.push_back(output);

		for (const auto &[uName, uTexture] : Renderer3D::GetSettings().InternalTextures)
		{
			m_TextureNames.push_back(uName.c_str());
		}
	}

	void RenderNodeTexture::Start()
	{
		for (const auto& link : links)
		{
			if (link->pInput->type == RenderAttributeType::Texture)
			{
				if (RenderNodePass *passNode = dynamic_cast<RenderNodePass *>(link->pInput->pNode))
				{
					uint32_t index = std::any_cast<uint32_t>(link->pInput->data);
					auto &[passTex, bindId] = passNode->renderPass->GetSpecification().textures[index];
					passTex = texture;
				}
			}
		}
	}

	void RenderNodeTexture::DrawNode()
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(62, 189, 100, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(100, 181, 124, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(100, 181, 124, 255));

		ImNodes::BeginNode(id);
		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("Texture node");
		ImNodes::EndNodeTitleBar();

		ImGui::Checkbox("Use internal texture", &m_UseInternalTextures);
		ImGui::Text("Texture:");

		if (m_UseInternalTextures)
		{
			ImGui::PushItemWidth(150.f);
			if (ImGui::Combo("##textures", &m_CurrentlySelectedTexture, m_TextureNames.data(), m_TextureNames.size()))
			{
				texture = Renderer3D::GetSettings().InternalTextures[m_TextureNames[m_CurrentlySelectedTexture]];
				m_SelectedTextureName = m_TextureNames[m_CurrentlySelectedTexture];
			}
			ImGui::PopItemWidth();
		}
		else
		{
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
		LP_SERIALIZE_PROPERTY(usingInternal, m_UseInternalTextures, out);
		LP_SERIALIZE_PROPERTY(selectedTexture, m_SelectedTextureName, out);

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
		LP_DESERIALIZE_PROPERTY(usingInternal, m_UseInternalTextures, node, false);
		m_SelectedTextureName = node["selectedTexture"].as<std::string>();

		if (!m_UseInternalTextures)
		{
			std::string texPath = node["path"].as<std::string>();
			if (!texPath.empty())
			{
				texture = ResourceCache::GetAsset<Texture2D>(texPath);
			}
		}
		else
		{
			texture = Renderer3D::GetSettings().InternalTextures[m_SelectedTextureName];
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