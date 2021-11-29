#include "lppch.h"
#include "WindowsWindow.h"

#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Event/KeyEvent.h"
#include "Lamp/Event/MouseEvent.h"

namespace Lamp
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		LP_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		LP_PROFILE_FUNCTION();
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		LP_PROFILE_FUNCTION();
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.VSync = props.IsVSync;

		if (!glfwInit())
		{
			LP_CORE_ERROR("Could not initialize GLFW");
		}

		glfwSetErrorCallback(GLFWErrorCallback);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

#if defined(LP_DEBUG)
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		}
		else if (Renderer::GetAPI() == RendererAPI::API::DX11 || Renderer::GetAPI() == RendererAPI::API::Vulkan)
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
#endif
		//Create the window
		m_pWindow = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);

		m_pContext = GraphicsContext::Create(m_pWindow);
		m_pContext->Initialize();

		glfwSetWindowUserPointer(m_pWindow, &m_Data);
		SetIsVSync(m_Data.VSync);

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

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	void WindowsWindow::Update(Timestep ts)
	{
		LP_PROFILE_FUNCTION();
		glfwPollEvents();
		m_pContext->SwapBuffers();
	}

	void WindowsWindow::Maximize()
	{
		glfwMaximizeWindow(m_pWindow);
	}

	inline void WindowsWindow::SetIsVSync(bool state)
	{
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			if (state)
			{
				glfwSwapInterval(1);
			}
			else
			{
				glfwSwapInterval(0);
			}
		}

		m_Data.VSync = state;
	}

	void WindowsWindow::ShowCursor(bool state)
	{
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
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
	void WindowsWindow::SetSize(const glm::vec2& size)
	{
		glfwSetWindowSize(m_pWindow, (uint32_t)size.x, (uint32_t)size.y);

		WindowResizeEvent resize((uint32_t)size.x, (uint32_t)size.y);
		Lamp::Application::Get().OnEvent(resize);
	}
}