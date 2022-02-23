#pragma once

#include "Lamp/Layer/Layer.h"

#include "Lamp/Event/Event.h"
#include "Lamp/Event/MouseEvent.h"

#include "Lamp/Event/KeyEvent.h"
#include "Lamp/Event/ApplicationEvent.h"

#include "imgui.h"

namespace Lamp
{
	class RenderPipeline;
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnDetach() override;
		void OnAttach() override;

		void Begin();
		void End();

		static ImGuiLayer* Create();

	private:
		float m_time = 0.f;
		ImFont* m_font;

		VkDescriptorPool m_descriptorPool;
	};
}