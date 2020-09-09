#pragma once

#include <Lamp.h>
#include "Lamp/Rendering/Cameras/PerspectiveCameraController.h"

namespace Lamp
{
	class GameBase
	{
	public:
		virtual void OnStart() = 0;
		virtual void OnEvent(Event& e) = 0;
	};
}