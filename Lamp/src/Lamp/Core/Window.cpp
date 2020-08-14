#include "lppch.h"
#include "Window.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Event/KeyEvent.h"
#include "Lamp/Event/MouseEvent.h"

namespace Lamp
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		LP_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window::Window(WindowProps& props)
	{
		Initialize(props);
	}

	Window::~Window()
	{
		glfwTerminate();
	}

	void Window::Initialize(const WindowProps & props)
	{
		if (!glfwInit())
		{
			LP_CORE_ERROR("Could not initialize GLFW");
		}

		glfwSetErrorCallback(GLFWErrorCallback);
		glfwWindowHint(GLFW_SAMPLES, 4);
		//Create the window
		m_pWindow = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);
		if (!m_pWindow)
		{
			glfwTerminate();
			LP_CORE_ERROR("Could not create window!");
		}

		//Set the current context
		glfwMakeContextCurrent(m_pWindow);

		//Load all OpenGL function pointers
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			LP_CORE_ERROR("Failed to load OpenGL!");
		}

		LP_CORE_INFO("OpenGL Info:");
		LP_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		LP_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		LP_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));
		LP_CORE_INFO("Sound initialized!");

		glfwSetWindowUserPointer(m_pWindow, &m_Data);
		SetIsVSync(props.IsVSync);

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.VSync = props.IsVSync;

		//Set GLFW callbacks
		glfwSetWindowSizeCallback(m_pWindow, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

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
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_pWindow, [](GLFWwindow* pWindow, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(pWindow);
			
			MouseScrolledEvent event((float)xOffset, (float)yOffset);
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

	void Window::Update(Timestep ts)
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

	void Window::ShowCursor(bool state)
	{
		if (!state)
		{
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else 
		{
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}
