#include "lppch.h"
#include "Framebuffer.h"

#include "Lamp/Rendering/Renderer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Lamp
{
Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
{
    switch (Renderer::GetAPI())
    {
    case RendererAPI::API::None:
        LP_CORE_ASSERT(false, "None is not supported!");
        return nullptr;
    case RendererAPI::API::OpenGL:
        return CreateRef<OpenGLFramebuffer>(spec);
    }

    return nullptr;
}
}