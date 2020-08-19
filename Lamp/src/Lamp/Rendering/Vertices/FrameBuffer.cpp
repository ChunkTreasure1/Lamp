#include "lppch.h"
#include "FrameBuffer.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Lamp
{
	std::shared_ptr<FrameBuffer> FrameBuffer::Create(const uint32_t width, const uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "None is not supported!"); return nullptr;
			case RendererAPI::API::OpenGL: return std::make_shared<OpenGLFramebuffer>(width, height);
		}

		return nullptr;
	}
}