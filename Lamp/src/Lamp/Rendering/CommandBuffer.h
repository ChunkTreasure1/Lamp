#pragma once

#include "Lamp/Core/Core.h"

namespace Lamp
{
	class RenderPipeline;

	class CommandBuffer
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;
 
		virtual void* GetCurrentCommandBuffer() = 0;

		static Ref<CommandBuffer> Create(Ref<RenderPipeline> renderPipeline);

	private:
	};
}