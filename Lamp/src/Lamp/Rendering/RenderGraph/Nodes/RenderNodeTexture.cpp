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

		for (const auto& [uName, uTexture] : Renderer3D::GetSettings().InternalTextures)
		{
			m_TextureNames.push_back(uName.c_str());
		}
	}

	void RenderNodeTexture::Start()
	{
		for (const auto& link : links)
		{
			if (RenderNodePass* passNode = dynamic_cast<RenderNodePass*>(link->pInput->pNode))
			{
				GraphUUID id = std::any_cast<GraphUUID>(link->pInput->data);
				passNode->renderPass->GetSpecification().textures[id].first.texture = texture;
			}
		}
	}

	void RenderNodeTexture::DrawNode()
	{
		LP_PROFILE_FUNCTION();

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
			if (ImGui::Combo("##textures", &m_CurrentlySelectedTexture, m_TextureNames.data(), (int)m_TextureNames.size()))
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

		DrawAttributes();

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

		SerializeAttributes(out);
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

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
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