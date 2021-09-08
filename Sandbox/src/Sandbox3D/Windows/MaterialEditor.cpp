#include "MaterialEditor.h"

#include <imgui/imgui_stdlib.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Meshes/Materials/MaterialLibrary.h>

#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>

namespace Sandbox3D
{
	using namespace Lamp;

	static const std::filesystem::path s_assetsPath = "assets";

	MaterialEditor::MaterialEditor(std::string_view name)
		: BaseWindow(name)
	{
		m_Camera = CreateRef<PerspectiveCameraController>(60.f, 0.1f, 100.f);
		m_Camera->SetPosition({ 0.f, 0.f, 3.f });
		m_RenderFuncs.push_back(LP_EXTRA_RENDER(MaterialEditor::Render));

		{
			FramebufferSpecification main;
			main.Attachments =
			{
				{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge },
				{ FramebufferTextureFormat::DEPTH24STENCIL8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
			};
			main.ClearColor = { 0.1f, 0.1f, 0.1f, 1.f };
			main.Height = 1024;
			main.Width = 1024;

			m_Framebuffer = Framebuffer::Create(main);
		}

		m_MaterialModel = CreateRef<Model>();
		g_pEnv->pAssetManager->LoadModel("assets/models/sphere.lgf", m_MaterialModel.get());

		m_MaterialModel->SetModelMatrix(glm::mat4(1.f));
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
					MaterialLibrary::SaveMaterial(m_pSelectedMaterial->GetPath().string(), *m_pSelectedMaterial);
					LP_CORE_INFO("Saved materil {0}", m_pSelectedMaterial->GetName());
				}
			}
		}

		return false;
	}

	void MaterialEditor::UpdateMaterialView()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::Begin("Material View");

		ImVec2 panelSize = ImGui::GetContentRegionAvail();
		if (m_perspectiveSize != *((glm::vec2*)&panelSize))
		{
			m_Framebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			m_perspectiveSize = { panelSize.x, panelSize.y };

			m_Camera->UpdateProjection((uint32_t)panelSize.x, (uint32_t)panelSize.y);
		}

		uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
		ImGui::Image((ImTextureID)textureID, ImVec2{ panelSize.x, panelSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void MaterialEditor::UpdateProperties()
	{
		ImGui::Begin("Properties##Material");

		if (m_pSelectedMaterial)
		{
			ImGui::InputText("Name", &m_pSelectedMaterial->GetName());

			/////Shaders//////
			static std::vector<const char*> shaders;
			static int selectedShader = 0;
			if (shaders.empty())
			{
				for (auto& shader : Lamp::ShaderLibrary::GetShaders())
				{
					shaders.push_back(shader->GetName().c_str());
				}
			}

			for (int i = 0; i < shaders.size(); i++)
			{
				if (m_pSelectedMaterial->GetShader()->GetName() == shaders[i])
				{
					selectedShader = i;
				}
			}

			ImGui::Combo("Shader", &selectedShader, shaders.data(), shaders.size());
			if (m_pSelectedMaterial->GetShader() != Lamp::ShaderLibrary::GetShader(shaders[selectedShader]))
			{
				m_pSelectedMaterial->SetShader(Lamp::ShaderLibrary::GetShader(shaders[selectedShader]));
				for (auto& tex : m_pSelectedMaterial->GetTextures())
				{
					tex.second = Texture2D::Create("engine/textures/default/defaultTexture.png");
				}
			}
			/////////////////

			for (auto& tex : m_pSelectedMaterial->GetTextures())
			{
				if (tex.second)
				{
					if (ImGui::ImageButton((ImTextureID)tex.second->GetID(), { 128, 128 }, { 0, 1 }, { 1,0 }))
					{
						std::string path = FileDialogs::OpenFile("All (*.*)\0*.*\0");
						if (!path.empty())
						{
							tex.second = Texture2D::Create(path);
							tex.second->GetPath() = path;
						}
					}
					std::string dragDropPath = GetDragDropTarget();
					if (!dragDropPath.empty())
					{
						tex.second = Texture2D::Create(dragDropPath);
						tex.second->GetPath() = dragDropPath;
					}
				}
				else
				{
					if (ImGui::Button(std::string("Load##" + tex.first).c_str(), { 128, 128 }))
					{
						std::string path = FileDialogs::OpenFile("All (*.*)\0*.*\0");
						if (!path.empty())
						{
							tex.second = Texture2D::Create(path);
							tex.second->GetPath() = path;
						}
					}
					std::string dragDropPath = GetDragDropTarget();
					if (!dragDropPath.empty())
					{
						tex.second = Texture2D::Create(dragDropPath);
						tex.second->GetPath() = dragDropPath;
					}
				}

				std::string texPath;
				ImGui::InputText(tex.first.c_str(), tex.second ? &tex.second->GetPath() : &texPath);
			}
		}

		ImGui::End();
	}

	void MaterialEditor::UpdateMaterialList()
	{
		if (!m_IsOpen)
		{
			return;
		}

		ImGui::Begin("Material List");

		if (ImGui::Button("Create"))
		{

		}

		std::vector<std::filesystem::path> folders = FileSystem::GetMaterialFolders(s_assetsPath);
		int id = 0;

		if (ImGui::TreeNode("Assets"))
		{
			for (auto& folder : folders)
			{
				if (ImGui::TreeNode(folder.filename().string().c_str()))
				{
					for (const auto& entry : std::filesystem::directory_iterator(folder))
					{
						if (entry.path().extension() == ".mtl")
						{
							std::string filename = entry.path().stem().string();

							id++;
							ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
							ImGui::TreeNodeEx((void*)(intptr_t)id, nodeFlags, filename.c_str());
							if (ImGui::IsItemClicked())
							{
								m_pSelectedMaterial = &MaterialLibrary::GetMaterial(filename);
								if (m_MaterialModel)
								{
									m_MaterialModel->SetMaterial(*m_pSelectedMaterial);
								}
							}
						}
					}

					ImGui::TreePop();
				}

			}
			ImGui::TreePop();
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered())
		{
			m_pathOnRightClick = s_assetsPath;
			ImGui::OpenPopup("RightClick");
		}
		
		UpdateRightClickMenu();
		ImGui::End();
	}

	void MaterialEditor::UpdateRightClickMenu()
	{
		if (ImGui::BeginPopup("RightClick"))
		{
			ImGui::EndPopup();
		}
	}

	std::string MaterialEditor::GetDragDropTarget()

	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)pPayload->Data;
				std::filesystem::path p = std::filesystem::path("assets") / path;
				return p.string();
			}

			ImGui::EndDragDropTarget();
		}

		return "";
	}

	void MaterialEditor::Render()
	{
		if (!m_IsOpen)
		{
			return;
		}

		RenderPassSpecification pass;
		pass.Camera = m_Camera->GetCamera();
		pass.TargetFramebuffer = m_Framebuffer;

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });
		RenderCommand::Clear();

		m_Framebuffer->Bind();
		RenderCommand::Clear();

		Renderer3D::Begin(pass);
		if (m_MaterialModel)
		{
			m_MaterialModel->Render(-1);
		}
		Renderer3D::End();
		m_Framebuffer->Unbind();
	}

	void MaterialEditor::CreateNewMaterial()
	{
		std::filesystem::path matPath = s_assetsPath / "material.mtl";
		std::ofstream ofs(matPath);
		ofs << "<Material name=\"material\">\n";
		ofs << "	<albedo path=\"engine/textures/default/defaultTexture.png\"/>\n";
		ofs << "	<normal path=\"engine/textures/default/defaultTexture.png\"/>\n";
		ofs << "	<mro path=\"engine/textures/default/defaultTexture.png\"/>\n";
		ofs << "	<Shininess value=\"32.000\"/>\n";
		ofs << "	<Shader name=\"testPbr\" vertex=\"engine/shaders/3d/testPbr_vs.glsl\" fragment=\"engine/shaders/3d/testPbr_fs.glsl\"/>\n";
		ofs << "</Material>";
		ofs.close();
	}
}