#pragma once

#include "Lamp/Layer/Layer.h"

#include "Lamp/Event/Event.h"
#include "Lamp/Event/MouseEvent.h"

#include "Lamp/Event/KeyEvent.h"
#include "Lamp/Event/ApplicationEvent.h"

#include "imgui.h"

namespace Lamp
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void Begin();
		void End();
		
	private:
		float m_Time = 0.f;
		ImFont* m_pFont;
	};
}