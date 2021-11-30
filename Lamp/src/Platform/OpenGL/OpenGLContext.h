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
		virtual void Shutdown() override {}
		virtual void Update() override;
		virtual void* GetInstance() override { return nullptr; }
		virtual void* GetDevice() override { return nullptr; }

	private:
		GLFWwindow* m_pWindowHandle;
	};
}