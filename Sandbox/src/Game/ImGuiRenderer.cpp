//#include "lppch.h"
//#include "GameLayer.h"
//
//#include <Lamp/Brushes/Brush2D.h>
//#include <Lamp/Brushes/BrushManager.h>
//#include <Lamp/Level/LevelSystem.h>
//
//namespace Game
//{
//	void GameLayer::UpdatePerspective()
//	{
//		ImGui::Begin("Perspective");
//		{
//			myPerspectiveHover = ImGui::IsWindowHovered();
//			myCameraController->SetHasControl(myPerspectiveHover);
//
//			if (ImGui::BeginMenuBar())
//			{
//				if (ImGui::BeginMenu("Aspect ratio"))
//				{
//
//				}
//				ImGui::EndMenuBar();
//			}
//
//			myCameraController->SetAspectRatio(myAspectRatio);
//			ImVec2 pos = ImGui::GetCursorScreenPos();
//
//			float height = ImGui::GetWindowSize().x / myAspectRatio;
//
//			float offset = (ImGui::GetWindowSize().y - height) / 2;
//
//			myFrameBuffer->Update((uint32_t)height, (uint32_t)ImGui::GetWindowSize().y);
//
//			ImGui::GetWindowDrawList()->AddImage((void*)(uint64_t)myFrameBuffer->GetTexture(),
//				ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + offset),
//				ImVec2(pos.x + ImGui::GetWindowSize().x, pos.y + ImGui::GetWindowSize().y - offset),
//				ImVec2(0, 1),
//				ImVec2(1, 0));
//
//			myWindowSize = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
//		}
//		ImGui::End();
//	}
//
//	void GameLayer::UpdateBrushTool()
//	{
//		ImGui::Begin("Brush Tool");
//
//		if (Lamp::Input::IsMouseButtonPressed(0))
//		{
//			glm::vec2 tempWindowSize = glm::vec2(Lamp::Application::Get().GetWindow().GetWidth(), Lamp::Application::Get().GetWindow().GetHeight());
//
//			if (!myMouseCollided)
//			{
//				if (auto tempBrush = Lamp::LevelSystem::GetCurrentLevel()->GetBrushManager().GetBrushFromPoint(myCameraController->ScreenToWorldCoords(myMousePos, tempWindowSize)))
//				{
//					mySelected = tempBrush;
//				}
//				else
//				{
//					mySelected = nullptr;
//				}
//			}
//		}
//
//		if (mySelected)
//		{
//			glm::vec3 tempV = mySelected->GetPosition();
//
//			if (ImGui::CollapsingHeader("Brush Properties"))
//			{
//				float tempPos[3] = { tempV.x, tempV.y, tempV.z };
//				ImGui::InputFloat3("Position", tempPos, 3);
//				mySelected->SetPosition({ tempPos[0], tempPos[1], tempPos[2] });
//
//				
//
//				glm::vec3 tempR = mySelected->GetRotation();
//
//				float tempRot[3] = { glm::degrees(tempR.x), glm::degrees(tempR.y), glm::degrees(tempR.z) };
//				ImGui::InputFloat3("Rotation", tempRot, 3);
//				mySelected->SetRotation({ glm::radians(tempRot[0]), glm::radians(tempRot[1]), glm::radians(tempRot[2]) });
//
//				glm::vec3 tempS = mySelected->GetScale();
//
//				float tempScale[3] = { tempS.x, tempS.y, tempS.z };
//				ImGui::InputFloat3("Scale", tempScale, 3);
//				mySelected->SetScale({ tempScale[0], tempScale[1], tempScale[2] });
//
//				ImGui::Checkbox("Should Collide", &mySelected->m_ShouldCollide);
//
//				if (ImGui::Button("Remove Brush"))
//				{
//					Lamp::LevelSystem::GetCurrentLevel()->GetBrushManager().Remove(mySelected);
//
//					mySelected = nullptr;
//				}
//			}
//		}
//
//		if (ImGui::Button("Create Brush"))
//		{
//			if (m_SelectedFile.GetFileType() == Lamp::FileType::Texture)
//			{
//				mySelected = Lamp::LevelSystem::GetCurrentLevel()->GetBrushManager().Create(m_SelectedFile.GetPath());
//			}
//		}
//		ImGui::End();
//	}
//
//	void GameLayer::UpdateMainMenuBar()
//	{
//		if (ImGui::BeginMainMenuBar())
//		{
//			if (ImGui::BeginMenu("Level"))
//			{
//				if (ImGui::MenuItem("New"))
//				{
//
//				}
//
//				if (ImGui::MenuItem("Save"))
//				{
//					Lamp::LevelSystem::SaveLevel("engine/levels/" + Lamp::LevelSystem::GetCurrentLevel()->GetName() + ".level", Lamp::LevelSystem::GetCurrentLevel());
//				}
//
//				if (ImGui::BeginMenu("Load"))
//				{
//					Lamp::FileSystem::PrintLevelFiles(Lamp::FileSystem::GetAssetFolders());
//					ImGui::EndMenu();
//				}
//
//				ImGui::EndMenu();
//			}
//
//			if (ImGui::BeginMenu("Play"))
//			{
//				if (ImGui::MenuItem("Play"))
//				{
//					myIsEditing = false;
//					myCurrentCameraController = myPlayerController;
//					myCurrentCameraController->SetEditing(false);
//				}
//
//				if (ImGui::MenuItem("Edit"))
//				{
//					myIsEditing = true;
//					myCurrentCameraController = myCameraController;
//					myCurrentCameraController->SetEditing(true);
//				}
//
//				ImGui::EndMenu();
//			}
//
//			ImGui::EndMainMenuBar();
//		}
//	}
//
//	void GameLayer::UpdateAssetBrowser()
//	{
//		ImGui::Begin("Asset Browser");
//		{
//			//Asset browser
//			{
//				ImGui::BeginChild("Browser", ImVec2(200, ImGui::GetWindowSize().y * 0.85f), true);
//				{
//					std::vector<std::string> folders = Lamp::FileSystem::GetAssetFolders();
//
//					Lamp::FileSystem::PrintFoldersAndFiles(folders);
//				}
//				ImGui::EndChild();
//
//				ImGui::SameLine();
//				if (ImGui::BeginChild("Preview"))
//				{
//					if (m_SelectedFile.GetFileType() == Lamp::FileType::Texture)
//					{
//						Ref<Lamp::Texture2D> tempSelected = Lamp::Texture2D::Create(m_SelectedFile.GetPath());
//						ImGui::Image((void*)(uint64_t)tempSelected->GetID(), ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().x), ImVec2(0, 1), ImVec2(1, 0));
//					}
//				}
//				ImGui::EndChild();
//			}
//		}
//		ImGui::End();
//	}
//
//	void GameLayer::CreateDockspace()
//	{
//		static bool opt_fullscreen_persistant = true;
//		bool opt_fullscreen = opt_fullscreen_persistant;
//		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
//
//		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
//		// because it would be confusing to have two docking targets within each others.
//		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
//		if (opt_fullscreen)
//		{
//			ImGuiViewport* viewport = ImGui::GetMainViewport();
//			ImGui::SetNextWindowPos(viewport->Pos);
//			ImGui::SetNextWindowSize(viewport->Size);
//			ImGui::SetNextWindowViewport(viewport->ID);
//			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
//			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
//		}
//
//		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
//		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
//			window_flags |= ImGuiWindowFlags_NoBackground;
//
//		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
//		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
//		// all active windows docked into it will lose their parent and become undocked.
//		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
//		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
//		bool pp = true;
//		bool* p = &pp;
//		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
//		ImGui::Begin("DockSpace Demo", p, window_flags);
//		ImGui::PopStyleVar();
//
//		if (opt_fullscreen)
//			ImGui::PopStyleVar(2);
//
//		// DockSpace
//		ImGuiIO& io = ImGui::GetIO();
//		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
//		{
//			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
//			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
//		}
//
//
//		if (ImGui::BeginMenuBar())
//		{
//			if (ImGui::BeginMenu("File"))
//			{
//				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
//				// which we can't undo at the moment without finer window depth/z control.
//				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);
//
//				if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
//
//				ImGui::EndMenu();
//			}
//
//			ImGui::EndMenuBar();
//		}
//
//		ImGui::End();
//	}
//}