#include "Window.h"
#include "Errors.h"

namespace Lamp
{
	Window::Window()
	{}

	Window::~Window()
	{}

	//Initializes a window
	int Window::Initialize(std::string windowName, uint32_t windowWidth, uint32_t windowHeight, uint32_t currentFlags)
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
		m_pWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			windowWidth, windowHeight, SDL_WINDOW_OPENGL);

		if (m_pWindow == nullptr)
		{
			FatalError("SDL Window could not be created!");
			return -1;
		}

		//Create a OpenGL context and error check it
		SDL_GLContext glContext = SDL_GL_CreateContext(m_pWindow);
		if (glContext == nullptr)
		{
			FatalError("SDL_GL context could not be created!");
			return -1;
		}

		//Initialize GLEW and error check it
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			FatalError("Could not initialize GLEW!");
			return -1;
		}
		printf("***	OpenGL Version: %s ***", glGetString(GL_VERSION));

		glClearColor(0.0f, 0.0f, 1.0f, 1.f);

		//Turn on V-Sync
		SDL_GL_SetSwapInterval(1);

		//Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		return 0;
	}

	//Swaps the rendering buffer
	void Window::SwapBuffer()
	{
		//Swap the windows
		SDL_GL_SwapWindow(m_pWindow);
	}
}
