//#include "lppch.h"
//#include "GameLayer.h"
//
//#include <Lamp/Rendering/Renderer2D.h>
//#include <glm/glm.hpp>
//
//#include <Lamp/Physics/Collision.h>
//#include <Lamp/Brushes/BrushManager.h>
//#include <Lamp/Level/LevelSystem.h>
//
//#include "Gameplay/Components/PlayerComponent.h"
//#include "Gameplay/Components/Level/PlatformGeneratorComponent.h"
//
//namespace Game
//{
//	glm::vec2 GameLayer::myWindowSize;
//	Lamp::Brush2D* GameLayer::mySelected = nullptr;
//
//	GameLayer::GameLayer()
//		: Lamp::Layer("GameLayer"), myDockspaceID(0), m_SelectedFile(""), mySelectedAxis(Transform::None), myLastMousePos(0, 0)
//	{
//		auto tempLevel = Lamp::LevelSystem::LoadLevel("engine/levels/Level.level");
//
//		{
//			auto tempPlayer = tempLevel->GetEntityManager().Create();
//			auto tempComp = tempPlayer->GetOrCreateComponent<PlayerComponent>();
//			myPlayerController = tempComp->GetCamera();
//
//			auto tempPlatformGen = tempLevel->GetEntityManager().Create();
//			tempPlatformGen->GetOrCreateComponent<PlatformGeneratorComponent>()->SetPlayer(tempPlayer);
//
//			auto tempPlatform = tempLevel->GetEntityManager().Create();
//			tempPlatform->GetOrCreateComponent<PlatformComponent>();
//			tempPlatform->SetPosition({ 0.f, -1.f, 0.f });
//		}
//
//		myCameraController = new Lamp::OrthographicCameraController(Lamp::Application::Get().GetWindow().GetWidth() / Lamp::Application::Get().GetWindow().GetHeight());
//		myCameraController->SetEditing(true);
//		myCurrentCameraController = myCameraController;
//	}
//
//	void GameLayer::Update(Lamp::Timestep ts)
//	{
//		if (myIsEditing)
//		{
//			myCurrentCameraController->Update(ts);
//		
//			//UpdateInput(ts);
//		}
//		else
//		{
//			Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Update(ts);
//		}
//
//
//		Lamp::Renderer::SetClearColor(myClearColor);
//		Lamp::Renderer::Clear();
//
//		Lamp::Renderer::Clear();
//
//		Lamp::Renderer2D::Begin(myCurrentCameraController->GetCamera());
//
//
//		Lamp::LevelSystem::GetCurrentLevel()->GetBrushManager().Draw();
//		Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Draw();
//
//		//DrawGizmos();
//		Lamp::Renderer2D::End();
//	}
//
//	bool GameLayer::MouseMoved(Lamp::MouseMovedEvent& e)
//	{
//		myMousePos = glm::vec2(e.GetX(), e.GetY());
//		if (myMouseCollided && myMouseHold)
//		{
//			if (mySelected)
//			{
//				glm::vec2 tempWindowSize(Lamp::Application::Get().GetWindow().GetWidth(), Lamp::Application::Get().GetWindow().GetHeight());
//				glm::vec2 tempCoords = myCurrentCameraController->ScreenToWorldCoords(myMousePos, tempWindowSize);
//
//				mySelected->SetPosition(glm::vec3(0, (int)tempCoords.y, 0));
//			}
//		}
//		return false;
//	}
//
//	void GameLayer::UpdateInput(Lamp::Timestep ts)
//	{
//		if (Lamp::Input::IsMouseButtonPressed(0))
//		{
//			glm::vec2 tempWindowSize(Lamp::Application::Get().GetWindow().GetWidth(), Lamp::Application::Get().GetWindow().GetHeight());
//			myMouseCollided = CheckMouseCollision(myCurrentCameraController->ScreenToWorldCoords(myMousePos, tempWindowSize), 3.f);
//			myMouseHold = true;
//		}
//
//		if (Lamp::Input::IsMouseButtonReleased(0))
//		{
//			mySelectedAxis = Transform::None;
//			myMouseCollided = false;
//			myMouseHold = false;
//		}
//	}
//
//	void GameLayer::OnImGuiRender(Lamp::Timestep ts)
//	{
//		UpdateBrushTool();
//		UpdateMainMenuBar();
//		UpdateAssetBrowser();
//	}
//
//	void GameLayer::OnEvent(Lamp::Event& e)
//	{
//		Lamp::EventDispatcher tempDispatcher(e);
//		tempDispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(GameLayer::MouseMoved));
//
//		if (!myIsEditing)
//		{
//			Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().OnEvent(e);
//		}
//
//		if (myIsEditing)
//		{
//			myCurrentCameraController->OnEvent(e);
//		}
//	}
//
//	void GameLayer::OnItemClicked(Lamp::File& file)
//	{
//		m_SelectedFile = file;
//	}
//
//	void GameLayer::DrawGizmos()
//	{
//		if (mySelected)
//		{
//			//Lamp::Renderer2D::DrawLine(mySelected->GetPosition(), glm::vec2(1, 1));
//		}
//	}
//
//	bool GameLayer::CheckMouseCollision(const glm::vec2& mousePos, float lineWidth)
//	{
//		if (mySelected)
//		{
//			{
//				glm::vec4 tempRect(mySelected->GetPosition().x, mySelected->GetPosition().y, lineWidth, 1);
//
//				if ((mousePos.x > (tempRect.x - (tempRect.z / 2))) &&
//					(mousePos.x < (tempRect.x + (tempRect.z / 2))) &&
//
//					(mousePos.y > (tempRect.y - (tempRect.w / 2))) &&
//					(mousePos.y < (tempRect.y + (tempRect.w / 2))))
//				{
//					mySelectedAxis = Transform::XAxis;
//					return true;
//				}
//			}
//
//			{
//				glm::vec4 tempRect(mySelected->GetPosition().x, mySelected->GetPosition().y, 1, lineWidth);
//
//				if ((mousePos.x > (tempRect.x - (tempRect.z / 2))) &&
//					(mousePos.x < (tempRect.x + (tempRect.z / 2))) &&
//
//					(mousePos.y > (tempRect.y - (tempRect.w / 2))) &&
//					(mousePos.y < (tempRect.y + (tempRect.w / 2))))
//				{
//					mySelectedAxis = Transform::YAxis;
//					return true;
//				}
//			}
//		}
//		return false;
//	}
//}