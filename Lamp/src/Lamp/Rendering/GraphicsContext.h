#pragma once

#include "Lamp/Core/Core.h"

namespace Lamp
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void Update() = 0;
		virtual void* GetInstance() = 0;
		virtual void* GetDevice() = 0;
		
		static Ref<GraphicsContext> Create(void* data);
	};
}