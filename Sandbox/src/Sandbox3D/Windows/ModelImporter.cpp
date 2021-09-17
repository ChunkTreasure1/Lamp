#include "ModelImporter.h"

#include <Lamp/Rendering/RenderCommand.h>
#include <imgui/imgui.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Meshes/Materials/MaterialLibrary.h>
#include <imgui/imgui_stdlib.h>

#include <Platform/OpenGL/OpenGLFramebuffer.h>
#include <Lamp/AssetSystem/MeshImporter.h>
#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/AssetSystem/ResourceCache.h>

namespace Sandbox3D
{
	using namespace Lamp;

	ModelImporter::ModelImporter(std::string_view name)
		: BaseWindow(name)
	{
		m_Camera = CreateRef<PerspectiveCameraController>(60.f, 0.01f, 100.f);
		m_Camera->SetPosition({ -3.f, 2.f, 3.f });

		m_RenderFuncs.push_back(LP_EXTRA_RENDER(ModelImporter::Render));

		{
			FramebufferSpecification mainBuffer;
			mainBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge },
				{ FramebufferTextureFormat::DEPTH24STENCIL8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
			};
			mainBuffer.ClearColor = { 0.1f, 0.15f, 0.15f, 1.f };
			mainBuffer.Height = 720;
			mainBuffer.Width = 1280;

			m_Framebuffer = Lamp::Framebuffer::Create(mainBuffer);
		}
	}

	bool ModelImporter::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
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

		UpdatePerspective();
		UpdateProperties();
		UpdateMaterial();

		return false;
	}

	bool ModelImporter::Update(Lamp::AppUpdateEvent& e)
	{
		if (m_DefaultShader == nullptr)
		{
			m_DefaultShader = Lamp::ShaderLibrary::GetShader("pbrForward");
		}
		UpdateCamera(e.GetTimestep());

		return false;
	}

	std::string ModelImporter::GetDragDropTarget()
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

	void ModelImporter::UpdateCamera(Lamp::Timestep ts)
	{
		if (!m_IsOpen)
		{
			return;
		}

		if (Input::IsMouseButtonPressed(1) && (m_HoveringPerspective || m_RightMousePressed))
		{
			m_Camera->Update(ts);
		}
	}

	void ModelImporter::Render()
	{
		if (!m_IsOpen)
		{
			return;
		}

		RenderPassSpecification pass;
		pass.TargetFramebuffer = m_Framebuffer;
		pass.type = PassType::Forward;

		RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.1f));
		RenderCommand::Clear();

		m_Framebuffer->Bind();
		RenderCommand::Clear();

		Renderer3D::Begin(m_Camera->GetCamera());
		Renderer3D::BeginPass(pass);

		if (m_RenderSkybox)
		{
			Renderer3D::DrawSkybox();
		}

		if (m_pModelToImport.get() != nullptr)
		{
			m_pModelToImport->Render(0, glm::mat4(1.f), true);
		}
		if (m_RenderGrid)
		{
			RenderGrid();
		}

		Renderer3D::DrawRenderBuffer();

		Renderer3D::EndPass();
		Renderer3D::End();
		m_Framebuffer->Unbind();
	}

	void ModelImporter::OnEvent(Lamp::Event& e)
	{
		if (!m_IsOpen)
		{
			return;
		}

		if (Input::IsMouseButtonReleased(1))
		{
			m_Camera->SetHasControl(false);
			m_RightMousePressed = false;

			Application::Get().GetWindow().ShowCursor(true);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}

		if (Input::IsMouseButtonPressed(1) && (m_HoveringPerspective || m_RightMousePressed))
		{
			m_Camera->OnEvent(e);
			m_RightMousePressed = true;

			Application::Get().GetWindow().ShowCursor(false);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::ImGuiUpdateEvent>(LP_BIND_EVENT_FN(ModelImporter::UpdateImGui));
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(ModelImporter::Update));
	}

	void ModelImporter::RenderGrid()
	{
		Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.f), glm::vec3(5.f, 0.f, 0.f), 1.f);
		Renderer3D::DrawLine(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 5.f), 1.f);

		for (size_t x = 1; x <= 10; x++)
		{
			Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.5f * (float)x), glm::vec3(5.f, 0.f, 0.5f * (float)x), 1.f);
		}

		for (size_t x = 1; x <= 10; x++)
		{
			Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, -0.5f * (float)x), glm::vec3(5.f, 0.f, -0.5f * (float)x), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Renderer3D::DrawLine(glm::vec3(0.5f * (float)z, 0.f, -5.f), glm::vec3(0.5f * (float)z, 0.f, 5.f), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Renderer3D::DrawLine(glm::vec3(-0.5f * (float)z, 0.f, -5.f), glm::vec3(-0.5f * (float)z, 0.f, 5.f), 1.f);
		}
	}

	void ModelImporter::UpdatePerspective()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Import Perspective");
		{
			glm::vec2 windowPos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			m_HoveringPerspective = ImGui::IsWindowHovered();
			m_Camera->SetControlsEnabled(m_HoveringPerspective);

			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			if (m_PerspectiveSize != *((glm::vec2*)&panelSize))
			{
				m_Framebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
				m_PerspectiveSize = { panelSize.x, panelSize.y };

				m_Camera->UpdateProjection((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			}

			uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
			ImGui::Image((void*)(uint64_t)textureID, ImVec2{ panelSize.x, panelSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ModelImporter::UpdateProperties()
	{
		ImGui::Begin("Import Settings", &m_IsOpen);

		static std::string path = "";
		static std::string savePath = "";
		static std::string matName = "";

		if (ImGui::Button("Load##fbx"))
		{
			path = FileDialogs::OpenFile("FBX File (*.fbx)\0*.fbx\0");
			if (path != "" && std::filesystem::exists(path))
			{
				m_pModelToImport = MeshImporter::ImportMesh(path);

				savePath = path.substr(0, path.find_last_of('.'));
				savePath += ".lgf";
				m_pModelToImport->Path = savePath;

				for (auto& mat : m_pModelToImport->GetMaterials())
				{
					mat.second.SetShader(m_DefaultShader);
				
					for (auto& tex : mat.second.GetTextures())
					{
						tex.second = ResourceCache::GetAsset<Texture2D>("engine/textures/default/defaultTexture.png") ;
					}
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			savePath = Lamp::FileDialogs::SaveFile("Lamp Geometry (*.lgf)\0*.lgf\0");

			if (m_MaterialName.empty())
			{
				m_MaterialName = savePath.substr(savePath.find_last_of('\\') + 1, savePath.size() - 1);
			}

			if (savePath != "")
			{
				if (savePath.find(".lgf") == std::string::npos)
				{
					savePath += ".lgf";
				}

				m_pModelToImport->SetName(m_MaterialName);
				m_pModelToImport->Path = savePath;
				for (auto& mat : m_pModelToImport->GetMaterials())
				{
					mat.second.SetName(m_MaterialName + std::to_string(mat.first));
					if (!MaterialLibrary::IsMaterialLoaded(m_MaterialName + std::to_string(mat.first)))
					{
						std::string matPath = savePath.substr(0, savePath.find_last_of('\\') + 1);
						matPath += m_MaterialName + ".mtl";
						//Add material to library
						MaterialLibrary::SaveMaterial(matPath, m_pModelToImport->GetMaterial(mat.first));
						MaterialLibrary::AddMaterial(m_pModelToImport->GetMaterial(mat.first));
					}
				}

				g_pEnv->pAssetManager->SaveAsset(m_pModelToImport);
				path = "";
				savePath = "";
			}
		}
		ImGui::Text(("Source path: " + path).c_str());
		ImGui::Text(("Destination path: " + savePath).c_str());

		ImGui::Checkbox("Show Skybox", &m_RenderSkybox);
		ImGui::Checkbox("Show Grid", &m_RenderGrid);

		ImGui::Separator();
		ImGui::Text("Mesh settings");

		if (ImGui::DragFloat("Scale", &m_ImportSettings.MeshScale) && m_pModelToImport)
		{
		}
		
		static const char* meshDirections[] = {"Y+ up", "Y- up", "Z+ up", "Z- up", "X+ up", "X- up"};
		static int currentDirection = 0;
		ImGui::Combo("Up", &currentDirection, meshDirections, IM_ARRAYSIZE(meshDirections));

		ImGui::End();
	}

	void ModelImporter::UpdateMaterial()
	{
		ImGui::Begin("Materials");

		static std::vector<const char*> shaders;
		static std::unordered_map<std::string, std::string> paths;

		static int selectedItem = 0;

		shaders.clear();
		for (auto& shader : ShaderLibrary::GetShaders())
		{
			shaders.push_back(shader->GetName().c_str());
		}

		if (m_pModelToImport.get())
		{
			for (auto& mat : m_pModelToImport->GetMaterials())
			{
				if (ImGui::CollapsingHeader(mat.second.GetName().c_str()))
				{

				}
			}
		}

		ImGui::End();
	}
}