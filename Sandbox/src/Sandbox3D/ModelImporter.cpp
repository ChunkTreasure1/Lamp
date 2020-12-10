#include "ModelImporter.h"

#include <Lamp/Rendering/RenderCommand.h>
#include <imgui/imgui.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Meshes/GeometrySystem.h>

namespace Sandbox3D
{
	using namespace Lamp;

	ModelImporter::ModelImporter()
	{
		m_Camera = CreateRef<PerspectiveCameraController>(60.f, 0.1f, 100.f);
		m_FrameBuffer = FrameBuffer::Create(1280, 720);
	}

	void ModelImporter::Update()
	{
		if (!m_Open)
		{
			return;
		}

		if (m_DefaultShader == nullptr)
		{
			m_DefaultShader = Lamp::ShaderLibrary::GetShader("Illumn");
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Model Importer", &m_Open);
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("modelimporter");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();

		UpdatePerspective();
		UpdateProperties();
	}

	void ModelImporter::UpdateCamera(Lamp::Timestep ts)
	{
		if (!m_Open)
		{
			return;
		}
		m_Camera->Update(ts);
	}

	void ModelImporter::Render()
	{
		if (!m_Open)
		{
			return;
		}

		RenderPassInfo pass;
		pass.Camera = m_Camera->GetCamera();
		pass.IsShadowPass = false;
		pass.DirLight = g_pEnv->DirLight;
		pass.ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 0.1f);

		RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.1f));
		RenderCommand::Clear();

		m_FrameBuffer->Bind();
		RenderCommand::Clear();

		Renderer3D::Begin(pass);
		if (m_pModelToImport.get() != nullptr)
		{
			m_pModelToImport->Render();
		}
		RenderGrid();
		Renderer3D::End();
		m_FrameBuffer->Unbind();
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
			if (m_PerspectiveSize != *((glm::vec2*)& panelSize))
			{
				m_FrameBuffer->Update((uint32_t)panelSize.x, (uint32_t)panelSize.y);
				m_PerspectiveSize = { panelSize.x, panelSize.y };

				m_Camera->UpdateProjection((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			}

			uint32_t textureID = m_FrameBuffer->GetColorAttachment();
			ImGui::Image((void*)(uint64_t)textureID, ImVec2{ 1280, 720 }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ModelImporter::UpdateProperties()
	{
		ImGui::Begin("Import Properties", &m_Open);

		static std::string path = "";
		static std::string savePath = "";

		if (ImGui::Button("Load##fbx"))
		{
			path = FileDialogs::OpenFile("FBX File (*.fbx)\0*.fbx\0");
			if (path != "" && std::filesystem::exists(path))
			{
				m_pModelToImport = GeometrySystem::ImportModel(path);

				savePath = path.substr(0, path.find_last_of('.'));
				savePath += ".lgf";

				m_pModelToImport->GetMaterial().SetShader(m_DefaultShader);

				for (auto& tex : m_pModelToImport->GetMaterial().GetTextures())
				{
					tex.second = Lamp::Texture2D::Create("engine/textures/default/defaultTexture.png");
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			savePath = Lamp::FileDialogs::SaveFile("Lamp Geometry (*.lgf)\0*.lgf\0");
			Lamp::GeometrySystem::SaveToPath(m_pModelToImport, savePath);
			path = "";
			savePath = "";
			m_pModelToImport = nullptr;
		}
		ImGui::Text(("Source path: " + path).c_str());
		ImGui::Text(("Destination path: " + savePath).c_str());

		if (m_pModelToImport != nullptr)
		{
			static std::unordered_map<std::string, std::string> paths;
			for (auto& tex : m_pModelToImport->GetMaterial().GetTextures())
			{
				paths.emplace(tex.first, "");
			}
			//ImGui::InputText("Diffuse path:", &diffPath);
			ImGui::SameLine();
			for (auto& tex : m_pModelToImport->GetMaterial().GetTextures())
			{
				if (ImGui::Button((std::string("Load##") + tex.first).c_str()))
				{
					paths[tex.first] = Lamp::FileDialogs::OpenFile((tex.first + std::string(" Map (*.png ...)\0*.png\0*.jpg\0")).c_str());
				}
				m_pModelToImport->GetMaterial().SetTexture(tex.first, Lamp::Texture2D::Create(paths[tex.first]));
			}
			//ImGui::InputText("Specular path:", &specPath);

			//ImGui::InputText("Shader path:", &m_pModelToImport->GetMaterial().GetShader()->GetVertexPath());
		}

		ImGui::End();
	}
}