#include "Window.h"
#include "Lamp/Log.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Event/KeyEvent.h"
#include "Lamp/Event/MouseEvent.h"

namespace Lamp
{
	Window::Window(WindowProps& props)
	{
		Init(props);
	}

	Window::~Window()
	{
		delete m_pWindow;
		glfwTerminate();
	}

	void Window::Init(const WindowProps & props)
	{
		if (!glfwInit())
		{
			LP_CORE_ERROR("Could not initialize GLFW");
		}

		//Create the window
		m_pWindow = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
		if (!m_pWindow)
		{
			glfwTerminate();
			LP_CORE_ERROR("Could not create window!");
		}

		//Set the current context
		glfwMakeContextCurrent(m_pWindow);

		//Initialize GLEW and error check it
		GLenum error = glewInit();
		if (error != GLEW_OK)
		{
			LP_CORE_ERROR("Could not initialize GLEW!");
		}

		glfwSetWindowUserPointer(m_pWindow, &m_Data);
		SetIsVSync(props.IsVSync);

		//Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.VSync = props.IsVSync;

		//Set GLFW callbacks
		glfwSetWindowCloseCallback(m_pWindow, [](GLFWwindow* pWindow) 
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(pWindow);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_pWindow, [](GLFWwindow* pWindow, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(pWindow);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_pWindow, [](GLFWwindow* pWindow, uint32_t keyCode) 
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(pWindow);
			KeyTypedEvent event(keyCode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_pWindow, [](GLFWwindow* pWindow, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(pWindow);
			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(button);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(button);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_pWindow, [](GLFWwindow* pWindow, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer;
			
			MouseScrolledEvent event(xOffset, yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_pWindow, [](GLFWwindow* pWindow, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(pWindow);
			
			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	//Swaps the rendering buffer
	void Window::SwapBuffer()
	{
		//Swap the windows
		glfwSwapBuffers(m_pWindow);
	}

	void Window::Update()
	{
		glfwPollEvents();
		SwapBuffer();
	}

	inline void Window::SetIsVSync(bool state)
	{
		if (state)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}

		m_Data.VSync = state;
	}
}
