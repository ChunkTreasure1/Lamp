#include "lppch.h"
#include "Window.h"

#ifdef LP_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"	
#endif

namespace Lamp
{	
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef LP_PLATFORM_WINDOWS
		return std::make_unique<WindowsWindow>(props);
#endif
	}
}
