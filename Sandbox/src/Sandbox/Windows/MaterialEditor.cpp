#include "MaterialEditor.h"

#include <imgui/imgui_stdlib.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Mesh/Materials/MaterialLibrary.h>

#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/AssetSystem/ResourceCache.h>

#include <Lamp/Rendering/RenderGraph/RenderGraph.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeEnd.h>

namespace Sandbox
{
	using namespace Lamp;

	static const std::filesystem::path s_assetsPath = "assets";

	MaterialEditor::MaterialEditor(std::string_view name)
		: BaseWindow(name)
	{
		m_camera = CreateRef<PerspectiveCameraController>(60.f, 0.1f, 100.f);
		m_camera->SetPosition({ 0.f, 0.f, 3.f });

		m_RenderFuncs.emplace_back(LP_EXTRA_RENDER(MaterialEditor::Render));

		m_renderGraph = ResourceCache::GetAsset<RenderGraph>("assets/editor.rendergraph");
		m_renderGraph->Start();

		m_framebuffer = m_renderGraph->GetSpecification().endNode->framebuffer;
		m_materialModel = ResourceCache::GetAsset<Mesh>("assets/models/sphere.lgf");
	}

	void MaterialEditor::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(MaterialEditor::OnUpdate));
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(MaterialEditor::OnUpdateImGui));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(MaterialEditor::OnKeyPressed));
	}

	bool MaterialEditor::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		return false;
	}

	bool MaterialEditor::OnUpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_IsOpen)
		{
			return false;
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(m_Name.c_str(), &m_IsOpen);
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_Name.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();

		UpdateProperties();
		UpdateMaterialView();
		UpdateMaterialList();
		return false;
	}

	bool MaterialEditor::OnKeyPressed(Lamp::KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
			case LP_KEY_S:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);
				if (control && m_pSelectedMaterial)
				{
					g_pEnv->pAssetManager->SaveAsset(m_pSelectedMaterial);
					LP_CORE_INFO("Saved material {0}", m_pSelectedMaterial->GetName());
				}
			}
		}

		return false;
	}

	void MaterialEditor::UpdateMaterialView()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::Begin("Material View");

		if (m_renderGraph->GetSpecification().endNode->framebuffer)
		{
			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			if (m_perspectiveSize != *((glm::vec2*)&panelSize))
			{
				m_renderGraph->GetSpecification().endNode->framebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
				m_perspectiveSize = { panelSize.x, panelSize.y };

				m_camera->UpdateProjection((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			}

			uint32_t textureID = m_renderGraph->GetSpecification().endNode->framebuffer->GetColorAttachmentID(0);
			ImGui::Image((ImTextureID)textureID, ImVec2{ panelSize.x, panelSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* wPath = (const wchar_t*)pPayload->Data;
					std::filesystem::path path(wPath);

					AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
					if (type == Lamp::AssetType::Material)
					{
						//Todo: set material
					}
				}

				ImGui::EndDragDropTarget();
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void MaterialEditor::UpdateProperties()
	{
		ImGui::Begin("Properties##Material");

		if (!m_materialModel)
		{
			return;
		}

		if (!m_pSelectedMaterial)
		{
			m_pSelectedMaterial = m_materialModel->GetMaterial(0);
		}

		static std::string tempName = "";
		tempName = m_pSelectedMaterial->GetName();

		if (ImGui::InputText("Name", &tempName))
		{
			m_pSelectedMaterial->SetName(tempName);
		}

		for (auto& tex : m_pSelectedMaterial->GetTextures())
		{
			if (ImGui::ImageButton(ImTextureID(tex.second->GetID()), { 128, 128 }))
			{
				std::string path = FileDialogs::OpenFile("All (*.*)\0*.*\0");
				if (!path.empty())
				{
					Ref<Texture2D> newTex = ResourceCache::GetAsset<Texture2D>(path);
					if (newTex->IsValid())
					{
						tex.second = newTex;
					}
				}
			}

		}

		ImGui::End();
	}

	void MaterialEditor::UpdateMaterialList()
	{
		ImGui::Begin("Materials##matEd");

		auto& materials = MaterialLibrary::GetMaterials();
		int i = 0;
		for (auto& mat : materials)
		{
			std::string id = mat->GetName() + "###mat" + std::to_string(i);
			if (ImGui::Selectable(id.c_str()))
			{
				m_materialModel->SetMaterial(mat, 0);
				m_pSelectedMaterial = mat;
			}
			std::string popId = mat->GetName() + "##matPop" + std::to_string(i);
			if (ImGui::BeginPopupContextItem(popId.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::MenuItem("Remove"))
				{
				}

				ImGui::EndPopup();
			}

			i++;
		}

		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			if (ImGui::MenuItem("New"))
			{
				CreateNewMaterial();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void MaterialEditor::Render()
	{
		if (!m_IsOpen)
		{
			return;
		}

		if (m_materialModel)
		{
			for (const auto& mesh : m_materialModel->GetSubMeshes())
			{
				Renderer3D::SubmitMesh(glm::mat4(1.f), mesh, m_materialModel->GetMaterials()[mesh->GetMaterialIndex()]);
			}
		}

		m_renderGraph->Run(m_camera->GetCamera());
	}

	void MaterialEditor::CreateNewMaterial()
	{
		std::filesystem::path matPath = s_assetsPath / "material.mtl";
		std::ofstream ofs(matPath);
		ofs << "<Material name=\"material\">\n";
		ofs << "	<albedo path=\"engine/textures/default/defaultTexture.png\"/>\n";
		ofs << "	<normal path=\"engine/textures/default/defaultTexture.png\"/>\n";
		ofs << "	<mro path=\"engine/textures/default/defaultTexture.png\"/>\n";
		ofs << "	<Shader name=\"testPbr\" vertex=\"engine/shaders/3d/testPbr_vs.glsl\" fragment=\"engine/shaders/3d/testPbr_fs.glsl\"/>\n";
		ofs << "</Material>";
		ofs.close();
	}
}