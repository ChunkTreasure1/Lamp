#include "LayerView.h"

#include <imgui/imgui_stdlib.h>
#include <Lamp/Utility/UIUtility.h>
#include <Lamp/Core/Application.h>

#include <Lamp/AssetSystem/ResourceCache.h>

namespace Sandbox
{
	using namespace Lamp;

	LayerView::LayerView(std::string_view name)
		: BaseWindow(name)
	{
		m_entityIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/gizmos/gizmoEntity.png");
		m_brushIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/AssetIcons/iconMesh.png");

		m_visibleIconV = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/layerView/visible_v.png");
		m_visibleIconN = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/layerView/visible_n.png");

		m_lockedIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/layerView/locked.png");
		m_unlockedIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/layerView/unlocked.png");
	}

	void LayerView::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(LayerView::UpdateImGui));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(LayerView::Update));
	}

	bool LayerView::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();

		if (!m_IsOpen)
		{
			return false;
		}

		ImGui::Begin("Layers", &m_IsOpen);

		m_isFocused = ImGui::IsWindowFocused();

		const float imageSize = 20.f;
		const float imagePadding = 5.f;

		int startId = 0;
		bool isItemHovered = false;

		for (auto& layer : g_pEnv->pLevel->GetLayers())
		{
			std::string checkId = "###check" + std::to_string(layer.ID);

			if (ImGui::Checkbox(checkId.c_str(), &layer.Active))
			{
				for (auto obj : layer.Objects)
				{
					obj->SetIsActive(layer.Active);
				}
			}

			ImGui::SameLine();
			std::string id = m_renamingLayer == layer.ID ? "###layer" + std::to_string(layer.ID) : layer.Name + "###layer" + std::to_string(layer.ID);

			ImVec2 cursorPos = ImGui::GetCursorPos();
			bool open = UI::TreeNodeFramed(id, true, 0.f, { 0.f, 1.5f });
			if (ImGui::IsItemHovered())
			{
				isItemHovered = true;
			}

			if (UI::BeginPopup())
			{
				std::string renameText = "Rename layer '" + layer.Name + "'";
				if (ImGui::Selectable(renameText.c_str()))
				{
					m_renamingLayer = static_cast<int>(layer.ID);
				}

				if (ImGui::Selectable("Remove layer"))
				{
					g_pEnv->pLevel->RemoveLayer(layer.ID);

					ImGui::CloseCurrentPopup();
				}

				UI::EndPopup();
			}

			if (m_renamingLayer == layer.ID)
			{
				ImGui::SameLine();
				ImGui::SetCursorPosX(cursorPos.x + ImGui::GetTreeNodeToLabelSpacing());

				std::string renameId = "###rename" + std::to_string(layer.ID);
				ImGui::PushItemWidth(75.f);
				UI::ScopedColor background{ ImGuiCol_FrameBg, { 0.1f, 0.1f, 0.1f, 0.1f } };

				ImGui::InputTextString(renameId.c_str(), &layer.Name);
				if (m_renamingLayer != m_lastRenamingLayer)
				{
					ImGuiID widgetId = ImGui::GetCurrentWindow()->GetID(renameId.c_str());
					ImGui::SetFocusID(widgetId, ImGui::GetCurrentWindow());
					ImGui::SetKeyboardFocusHere(-1);
					m_lastRenamingLayer = m_renamingLayer;
				}
				if (!ImGui::IsItemFocused())
				{
					m_renamingLayer = -1;
					m_lastRenamingLayer = -1;
				}
				if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
				{
					m_renamingLayer = -1;
					m_lastRenamingLayer = -1;
				}

				ImGui::PopItemWidth();
			}

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
						ImGui::PushID(startId);
						static bool test = false;
						ImGui::TableNextRow();

						ImGui::TableNextColumn();
						uint32_t visibleIcon = obj->GetIsActive() ? m_visibleIconV->GetID() : m_visibleIconN->GetID();

						if (ImGui::ImageButton((ImTextureID)(visibleIcon), { imageSize, imageSize }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
						{
							obj->SetIsActive(!obj->GetIsActive());
						}
						ImGui::SameLine();
						
						uint32_t lockedIcon = obj->GetIsFrozen() ? m_lockedIcon->GetID() : m_unlockedIcon->GetID();
						if (ImGui::ImageButton((ImTextureID)(lockedIcon), { imageSize, imageSize }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
						{
							obj->SetIsFrozen(!obj->GetIsFrozen());
						}

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

						std::string id = obj->GetName() + "##" + obj->GetName() + std::to_string(obj->GetID());
						if (UI::ImageSelectable(icon->GetID(), id, obj->GetIsSelected()))
						{
							EditorObjectSelectedEvent e(obj);
							Application::Get().OnEvent(e);
						}

						if (ImGui::IsItemHovered())
						{
							isItemHovered = true;
						}

						bool objectDestroyed = false;

						if (UI::BeginPopup())
						{
							std::string removeText = "Remove '" + obj->GetName() + "'";
							if (ImGui::Selectable(removeText.c_str()))
							{
								obj->Destroy();
								objectDestroyed = true;
							}

							UI::EndPopup();
						}

						if (ImGui::BeginDragDropSource())
						{
							std::string dragDropText = "Moving " + obj->GetName();
							ImGui::TextUnformatted(dragDropText.c_str());
						
							const uint32_t values[2] = { layer.ID, obj->GetID() };
							ImGui::SetDragDropPayload("LAYER_OBJECT", values, sizeof(uint32_t) * 2, ImGuiCond_Once);
							ImGui::EndDragDropSource();
						}

						if (!objectDestroyed)
						{
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

						ImGui::PopID();
						startId++;
					}

					ImGui::EndTable();
				}

				UI::TreeNodePop();
			}
		}

		if (!isItemHovered && ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Add layer"))
			{
				g_pEnv->pLevel->AddLayer(ObjectLayer("New layer", (uint32_t)g_pEnv->pLevel->GetLayers().size(), true));
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::End();

		return false;
	}

	bool LayerView::Update(Lamp::AppUpdateEvent& e)
	{
		if (!m_isFocused)
		{
			return false;
		}

		if (Input::IsKeyPressed(LP_KEY_ENTER))
		{
			m_renamingLayer = -1;
			m_lastRenamingLayer = -1;
		}

		return false;
	}
}