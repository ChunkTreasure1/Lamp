#pragma once
#include "GLFW/glfw3.h"

namespace Lamp
{
	class OpenGLContext
	{
	public:
		OpenGLContext(GLFWwindow* pWindow);
		~OpenGLContext();

	private:
		GLFWwindow* m_pWindow;
	};
}