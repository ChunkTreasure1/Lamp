#include "lppch.h"
#include "RenderCommand.h"

namespace Lamp
{
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}