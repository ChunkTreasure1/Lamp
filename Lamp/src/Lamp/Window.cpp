#include "Window.h"
#include "Errors.h"

namespace Lamp
{
	Window::Window(WindowProps& props, uint32_t currentFlags)
	{
		//Get the window flags
		Uint32 flags = SDL_WINDOW_OPENGL;
		if (currentFlags & WINDOW_INVISIBLE)
		{
			flags |= SDL_WINDOW_HIDDEN;
		}
		if (currentFlags & WINDOW_FULLSCREEN)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		if (currentFlags & WINDOW_BORDERLESS)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		//Create a window and check it for errors
		m_pWindow = SDL_CreateWindow(props.Title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			props.Width, props.Height, SDL_WINDOW_OPENGL);

		if (m_pWindow == nullptr)
		{
			FatalError("SDL Window could not be created!");
		}

		//Create a OpenGL context and error check it
		SDL_GLContext glContext = SDL_GL_CreateContext(m_pWindow);
		if (glContext == nullptr)
		{
			FatalError("SDL_GL context could not be created!");
		}

		//Initialize GLEW and error check it
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			FatalError("Could not initialize GLEW!");

		}
		printf("***	OpenGL Version: %s ***\n", glGetString(GL_VERSION));

		SetIsVSync(props.IsVSync);

		//Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.IsVSync = props.IsVSync;
	}

	Window::~Window()
	{
		delete m_pWindow;
	}

	//Swaps the rendering buffer
	void Window::SwapBuffer()
	{
		//Swap the windows
		SDL_GL_SwapWindow(m_pWindow);
	}

	inline void Window::SetIsVSync(bool state)
	{
		if (state)
		{
			SDL_GL_SetSwapInterval(1);
		}
		else
		{
			SDL_GL_SetSwapInterval(0);
		}

		m_Data.IsVSync = state;
	}
}
