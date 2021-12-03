#pragma once

#include "Lamp/ImGui/ImGuiLayer.h"

namespace Lamp
{
	class OpenGLImGuiLayer : public ImGuiLayer
	{
	public:
		OpenGLImGuiLayer();
		~OpenGLImGuiLayer();

		void OnDetach() override;
		void OnAttach() override;

		void Begin() override;
		void End() override;

	private:
		float m_time = 0.f;
		ImFont* m_font;
	};
}