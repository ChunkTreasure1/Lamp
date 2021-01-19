#include "lppch.h"
#include "FrameBuffer.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Lamp
{
	Ref<FrameBuffer> FrameBuffer::Create(const uint32_t width, const uint32_t height, bool state)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return CreateRef<OpenGLFramebuffer>(width, height, state);
		}

		return nullptr;
	}
}