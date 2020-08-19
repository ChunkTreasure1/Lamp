#pragma once

#include "Lamp/Core/Core.h"

namespace Lamp
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Initialize() = 0;
		virtual void SwapBuffers() = 0;

		static Scope<GraphicsContext> Create(void* data);
	};
}