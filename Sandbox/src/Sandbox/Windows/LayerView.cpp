#include "LayerView.h"

#include <imgui/imgui_stdlib.h>
#include <Lamp/Utility/UIUtility.h>

#include <Lamp/AssetSystem/ResourceCache.h>

namespace Sandbox
{
	using namespace Lamp;

	LayerView::LayerView(std::string_view name)
		: BaseWindow(name)
	{
		m_entityIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/gizmos/gizmoEntity.png");
		m_brushIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/AssetIcons/iconMesh.png");

		m_visibleIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/layerView/visible.png");
		m_frozenIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/layerView/frozen.png");
	}

	void LayerView::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(LayerView::UpdateImGui));
	}

	bool LayerView::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();

		if (!m_IsOpen)
		{
			return false;
		}

		ImGui::Begin("Layers", &m_IsOpen);

		static std::string currentRightClick = "";
		static bool itemMenuOpen = false;
		static uint32_t idMenuOpen = 0;

		const float imageSize = 20.f;
		const float imagePadding = 10.f;

		int startId = 0;
		for (auto& layer : g_pEnv->pLevel->GetLayers())
		{
			std::string checkId = "###check" + std::to_string(layer.ID);
			ImGui::Checkbox(checkId.c_str(), &layer.Active);
			ImGui::SameLine();

			std::string id = layer.Name + "###layer" + std::to_string(layer.ID);

			bool open = UI::TreeNodeFramed(id, true, 0.f, { 0.f, 2.f });
			CollapsingHeaderAddons(currentRightClick, id, itemMenuOpen, idMenuOpen, layer.ID);

			const ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;

			if (open)
			{
				UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
				UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
				UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

				std::string id = "##table" + std::to_string(layer.ID);
				if (ImGui::BeginTable(id.c_str(), 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Settings", ImGuiTableColumnFlags_WidthFixed, imageSize * 2 + imagePadding);
					ImGui::TableSetupColumn("Name");
					ImGui::TableSetupColumn("Type");
					ImGui::TableHeadersRow();

					for (auto obj : layer.Objects)
					{
						static bool test = false;
						ImGui::TableNextRow();

						ImGui::TableNextColumn();
						ImGui::ImageButton((ImTextureID)(m_visibleIcon->GetID()), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0);
						ImGui::SameLine();
						ImGui::ImageButton((ImTextureID)(m_frozenIcon->GetID()), { imageSize, imageSize }, { 0.f, 0.f }, { 1.f, 1.f }, 0);

						ImGui::TableNextColumn();
						Ref<Texture2D> icon;
						if (typeid(*obj) == typeid(Brush))
						{
							icon = m_brushIcon;
						}
						else
						{
							icon = m_entityIcon;
						}
						UI::ImageTreeNode(icon->GetID(), (void*)(intptr_t)startId, nodeFlags, obj->GetName().c_str());

						ImGui::TableNextColumn();
						if (typeid(*obj) == typeid(Brush))
						{
							ImGui::TextDisabled("Brush");
						}
						else
						{
							ImGui::TextDisabled("Entity");
						}
					}

					ImGui::EndTable();
				}

				UI::TreeNodePop();
			}


			//if (ImGui::TreeNode(id.c_str()))
			//{
			//	CollapsingHeaderAddons(currentRightClick, id, itemMenuOpen, idMenuOpen, layer.ID);

			//	std::string inputId("Name"); inputId += "###input" + std::to_string(layer.ID);
			//	ImGui::InputText(inputId.c_str(), &layer.Name);

			//	for (auto& obj : layer.Objects)
			//	{
			//		startId++;
			//		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			//		Ref<Texture2D> icon;
			//		if (auto* brush = dynamic_cast<Lamp::Brush*>(obj))
			//		{
			//			icon = m_BrushIcon;
			//		}
			//		else
			//		{
			//			icon = m_EntityIcon;
			//		}

			//		UI::ImageTreeNodeEx(icon->GetID(), (void*)(intptr_t)startId, nodeFlags, obj->GetName().c_str());
			//		if (ImGui::BeginDragDropSource())
			//		{
			//			const uint32_t values[2] = { layer.ID, obj->GetID() };
			//			ImGui::SetDragDropPayload("LAYER_OBJECT", values, sizeof(uint32_t) * 2, ImGuiCond_Once);
			//			ImGui::EndDragDropSource();
			//		}

			//		if (ImGui::IsItemClicked())
			//		{
			//			if (m_pSelectedObject)
			//			{
			//				m_pSelectedObject->SetIsSelected(false);
			//			}

			//			m_pSelectedObject = obj;

			//			if (m_pSelectedObject)
			//			{
			//				m_pSelectedObject->SetIsSelected(true);
			//			}
			//		}
			//	}

			//	ImGui::TreePop();
			//}
			//else
			//{
			//	CollapsingHeaderAddons(currentRightClick, id, itemMenuOpen, idMenuOpen, layer.ID);
			//}
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered() && !itemMenuOpen)
		{
			ImGui::OpenPopup("LV_Main");
		}

		UpdateMainClickMenu();
		UpdateItemClickMenu(currentRightClick, itemMenuOpen, idMenuOpen);

		ImGui::End();

		return false;
	}

	void LayerView::UpdateMainClickMenu()
	{
		if (ImGui::BeginPopup("LV_Main"))
		{
			if (ImGui::Selectable("Add"))
			{
				g_pEnv->pLevel->AddLayer(ObjectLayer("New layer", (uint32_t)g_pEnv->pLevel->GetLayers().size(), true));
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void LayerView::UpdateItemClickMenu(const std::string& id, bool& open, uint32_t& layerId)
	{
		if (ImGui::BeginPopup(id.c_str()))
		{
			if (id == "")
			{
				open = false;
				ImGui::CloseCurrentPopup();
			}

			if (ImGui::Selectable("Remove"))
			{
				g_pEnv->pLevel->RemoveLayer(layerId);

				layerId = 0;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		else
		{
			open = false;
		}
	}

	void LayerView::CollapsingHeaderAddons(std::string& currRightClick, const std::string& id, bool& open, uint32_t& layerId, const uint32_t& currId)
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
		{
			ImGui::OpenPopup(id.c_str());
			open = true;
			currRightClick = id;
			layerId = currId;
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("LAYER_OBJECT"))
			{
				const uint32_t* values = (const uint32_t*)pPayload->Data;
				if (values[0] != currId)
				{
					g_pEnv->pLevel->MoveObjectToLayer(values[0], currId, values[1]);
				}
			}

			ImGui::EndDragDropTarget();
		}
	}
}