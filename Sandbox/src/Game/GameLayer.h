//#pragma once
//
//#include <Lamp.h>
//
//#include "imgui/imgui.h"
//#include "imgui/imgui_internal.h"
//
//#include <glm/gtc/type_ptr.hpp>
//#include <Lamp/Brushes/Brush2D.h>
//
//namespace Game
//{
//	enum class Transform
//	{
//		XAxis,
//		YAxis,
//		None
//	};
//
//	class GameLayer : public Lamp::Layer
//	{
//	public:
//		GameLayer();
//
//		virtual void Update(Lamp::Timestep ts) override;
//		virtual void OnImGuiRender(Lamp::Timestep ts) override;
//		virtual void OnEvent(Lamp::Event& e) override;
//		virtual void OnItemClicked(Lamp::File& file) override;
//
//	private:
//		void CreateDockspace();
//		bool MouseMoved(Lamp::MouseMovedEvent& e);
//		void UpdateInput(Lamp::Timestep ts);
//
//		//ImGui
//		void UpdatePerspective();
//		void UpdateBrushTool();
//		void UpdateMainMenuBar();
//		void UpdateAssetBrowser();
//
//		//Gizmos
//		void DrawGizmos();
//		bool CheckMouseCollision(const glm::vec2& mousePos, float lineWidth);
//
//	private:
//		Lamp::OrthographicCameraController* myCurrentCameraController;
//		Lamp::OrthographicCameraController* myCameraController;
//		Lamp::OrthographicCameraController* myPlayerController;
//
//		//---------------Editor-----------------
//		glm::vec4 myClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
//		glm::vec2 myMousePos;
//		glm::vec2 myLastMousePos;
//		ImGuiID myDockspaceID;
//
//		Lamp::File m_SelectedFile;
//		bool myIsEditing = true;
//		bool myMouseCollided = false;
//		bool myMouseHold = false;
//
//		Transform mySelectedAxis;
//
//		//Perspective
//		const float myAspectRatio = 1.7f;
//		Ref<Lamp::FrameBuffer> myFrameBuffer;
//
//		//Windows
//		bool myPerspectiveHover = false;
//		//--------------------------------------
//
//	public:
//		static const glm::vec2& GetWindowSize() { return myWindowSize; }
//		static Lamp::Brush2D* mySelected;
//
//	private:
//		static glm::vec2 myWindowSize;
//	};
//}