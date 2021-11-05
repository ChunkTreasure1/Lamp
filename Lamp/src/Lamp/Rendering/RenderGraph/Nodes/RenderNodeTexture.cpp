#include "lppch.h"
#include "RenderNodeTexture.h"

#include "RenderNodePass.h"
#include "RenderNodeCompute.h"
#include "Lamp/AssetSystem/ResourceCache.h"
#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Utility/UIUtility.h"

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

		for (const auto& [uName, uTexture] : Renderer::s_pSceneData->internalTextures)
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
				const_cast<RenderPassSpecification&>(passNode->renderPass->GetSpecification()).textures[id].first.texture = texture;
			}
			else if (RenderNodeCompute* computeNode = dynamic_cast<RenderNodeCompute*>(link->pInput->pNode))
			{
				//computeNode->texture = texture;
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

		const float maxOffset = 100.f;

		ImGui::PushID(("texNode" + std::to_string(id)).c_str());
		uint32_t stackId = 0;

		float offset = maxOffset - ImGui::CalcTextSize("Internal texture").x;
		ImGui::Text("Internal texture");

		ImGui::SameLine();
		UI::ShiftCursor(offset, 0.f);
		ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &m_UseInternalTextures);

		if (m_UseInternalTextures)
		{
			ImGui::PushItemWidth(128.f);
			if (ImGui::Combo("##textures", &m_CurrentlySelectedTexture, m_TextureNames.data(), (int)m_TextureNames.size()))
			{
				texture = Renderer::s_pSceneData->internalTextures[m_TextureNames[m_CurrentlySelectedTexture]];
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

		DrawAttributes(inputs, outputs);

		ImNodes::EndNode();

		ImGui::PopID();

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
			texture = Renderer::s_pSceneData->internalTextures[m_SelectedTextureName];
		}

		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}

	void RenderNodeTexture::GetTexture()
	{
		std::filesystem::path path = FileDialogs::OpenFile("All (*.*)\0*.*\0");
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