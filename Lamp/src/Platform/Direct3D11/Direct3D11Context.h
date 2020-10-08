#pragma once

#include "Lamp/Rendering/GraphicsContext.h"
#include <GLFW/glfw3.h>
#include <d3d11.h>
#include <wrl.h>

namespace Lamp
{
	class Direct3D11Context : public GraphicsContext
	{
	public:
		Direct3D11Context(GLFWwindow* windowHandle);
		~Direct3D11Context();

		virtual void Initialize() override;
		virtual void SwapBuffers() override;

		//Getting
		inline Microsoft::WRL::ComPtr<ID3D11Device>& GetDevice() { return m_pDevice; }
		inline Microsoft::WRL::ComPtr<IDXGISwapChain>& GetSwapChain() { return m_pSwapChain; }
		inline Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetDeviceContext() { return m_pContext; }
		inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& GetRenderTarget() { return m_pRenderTarget; }
		inline Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& GetDepthView() { return m_pDepthView; }

	private:
		GLFWwindow* m_pWindowHandle;

		HWND m_WindowsHandle;
		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_pDepthView;
	};
}