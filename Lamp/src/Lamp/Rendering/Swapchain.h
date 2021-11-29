#pragma once

#include "Lamp/Core/Core.h"

#include <cstdint>

namespace Lamp
{
	class GraphicsContext;
	class Swapchain
	{
	public:
		virtual ~Swapchain() = default;

		virtual void InitializeSurface(void* window) = 0;
		virtual void Shutdown() = 0;

		virtual void Invalidate(uint32_t& width, uint32_t& height) = 0;
		virtual void BeginFrame() = 0;
		virtual void Present() = 0;

		virtual void OnResize(uint32_t width, uint32_t height) = 0;
	
		static Scope<Swapchain> Create(Ref<GraphicsContext> context, void* device);
	};
}