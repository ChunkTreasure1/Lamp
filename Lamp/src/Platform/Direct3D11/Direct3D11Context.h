#pragma once

#include "Lamp/Rendering/GraphicsContext.h"
#include <GLFW/glfw3.h>
#include <d3d11.h>

namespace Lamp
{
	class Direct3D11Context : public GraphicsContext
	{
	public:
		Direct3D11Context(GLFWwindow* windowHandle);
		~Direct3D11Context();

		virtual void Initialize() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_pWindowHandle;

		HWND m_WindowsHandle;
		ID3D11Device* m_pDevice;
		IDXGISwapChain* m_pSwapChain;
		ID3D11DeviceContext* m_pContext;
	};
}