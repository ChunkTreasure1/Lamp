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
		virtual void Begin() = 0;
		virtual void End() = 0;

		static ImGuiLayer* Create();
	};
}