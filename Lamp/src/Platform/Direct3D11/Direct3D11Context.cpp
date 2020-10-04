#include "lppch.h"
#include "Direct3D11Context.h"

#include <GLFW/glfw3native.h>

#define GLFW_EXPOSE_NATIVE_WIN32

namespace wrl = Microsoft::WRL;

namespace Lamp
{
	Direct3D11Context::Direct3D11Context(GLFWwindow* windowHandle)
		: m_pWindowHandle(windowHandle), m_pContext(nullptr), m_pDevice(nullptr), m_pSwapChain(nullptr),
		m_pRenderTarget(nullptr), m_WindowsHandle(glfwGetWin32Window(windowHandle))
	{
#ifdef LP_ENABLE_ASSERTS
		LP_CORE_ASSERT(windowHandle, "WindowHandle is null!");
#endif
	}

	Direct3D11Context::~Direct3D11Context()
	{
	}

	void Direct3D11Context::Initialize()
	{
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 0;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = m_WindowsHandle;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		//Create device and fron/back buffers, also creates swap chain and render context
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, nullptr, &m_pContext);

		//Gain access to texture subresource in swap chain
		
		wrl::ComPtr<ID3D11Resource> pBackBuffer = nullptr;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer);
		m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pRenderTarget);
	}

	void Direct3D11Context::SwapBuffers()
	{
		m_pSwapChain->Present(1u, 0u);
	}
}