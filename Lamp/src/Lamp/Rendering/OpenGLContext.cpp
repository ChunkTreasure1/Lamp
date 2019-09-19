#include "lppch.h"
#include "OpenGLContext.h"

#include "GL/glew.h"

#include "Lamp/Core.h"

namespace Lamp
{
	OpenGLContext::OpenGLContext(GLFWwindow* pWindow)
		: m_pWindow(pWindow)
	{
		glfwMakeContextCurrent(pWindow);

		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			LP_CORE_ASSERT(false, "Could not initialze GLEW!");
		}
	}

	OpenGLContext::~OpenGLContext()
	{
		glfwTerminate();
	}
}