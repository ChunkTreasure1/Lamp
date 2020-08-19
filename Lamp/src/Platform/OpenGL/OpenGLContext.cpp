#include "lppch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Lamp
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_pWindowHandle(windowHandle)
	{
		LP_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Initialize()
	{
		glfwMakeContextCurrent(m_pWindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		LP_CORE_ASSERT(status, "Failed to initialize Glad!");

		LP_CORE_INFO("OpenGL Info:");
		LP_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		LP_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		LP_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));        
	
#ifdef LP_ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;

		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);

		LP_CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Lamp requires minimun OpenGL 4.5!");
#endif
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_pWindowHandle);
	}
}