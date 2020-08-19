#pragma once

#include "Lamp/Rendering/GraphicsContext.h"

struct GLFWwindow;

namespace Lamp
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Initialize() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_pWindowHandle;
	};
}