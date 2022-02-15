#include "lppch.h"
#include "WindowsWindow.h"


#include "Lamp/Rendering/Swapchain.h"

#include "Lamp/Event/KeyEvent.h"
#include "Lamp/Event/MouseEvent.h"
#include "Lamp/Core/Application.h"

#include "Platform/Vulkan/VulkanAllocator.h"

#include <stb/stb_image.h>

namespace Lamp
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		LP_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		LP_PROFILE_FUNCTION();
		Initialize(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		LP_PROFILE_FUNCTION();
		Shutdown();
	}

	void WindowsWindow::Initialize(const WindowProps& props)
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

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//Create the window
		m_pWindow = glfwCreateWindow(props.Width, props.Height, props.Title.c_str(), NULL, NULL);

		m_pContext = GraphicsContext::Create(m_pWindow);
		m_pContext->Initialize();

		m_swapchain = Swapchain::Create(m_pContext->GetInstance(), m_pContext->GetDevice());
		m_swapchain->InitializeSurface(m_pWindow);
		m_swapchain->Invalidate(m_Data.Width, m_Data.Height);

		glfwSetWindowUserPointer(m_pWindow, &m_Data);
		SetIsVSync(m_Data.VSync);

		{
			GLFWimage icon;
			int channels;
			icon.pixels = stbi_load("engine/textures/icon.png", &icon.width, &icon.height, &channels, 4);
			glfwSetWindowIcon(m_pWindow, 1, &icon);
			stbi_image_free(icon.pixels);
		}

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
		m_swapchain->Shutdown();
		m_pContext->Shutdown();
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	void WindowsWindow::Update(Timestep ts)
	{
		LP_PROFILE_FUNCTION();
		m_pContext->Update();
		m_swapchain->Present();
		glfwPollEvents();
	}

	void WindowsWindow::Maximize()
	{
		glfwMaximizeWindow(m_pWindow);
	}

	void WindowsWindow::OnResize(uint32_t width, uint32_t height)
	{
		m_swapchain->OnResize(width, height);
	}

	inline void WindowsWindow::SetIsVSync(bool state)
	{
		m_Data.VSync = state;
	}

	void WindowsWindow::ShowCursor(bool state)
	{
		static std::mutex lockMutex;
		std::lock_guard lock{ lockMutex };

		if (!state)
		{
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void WindowsWindow::SetSize(const glm::vec2& size)
	{
		glfwSetWindowSize(m_pWindow, (uint32_t)size.x, (uint32_t)size.y);
		m_swapchain->OnResize((uint32_t)size.x, (uint32_t)size.y);

		WindowResizeEvent resize((uint32_t)size.x, (uint32_t)size.y);
		Lamp::Application::Get().OnEvent(resize);
	}
}