#pragma once

#include <Lamp.h>

#include "Lamp/GraphKey/NodeRegistry.h"

namespace Lamp
{
	class GameBase
	{
	public:
		virtual void OnStart() = 0;
		virtual void OnStop() = 0;

		virtual void OnEvent(Event& e) = 0;
	};
}