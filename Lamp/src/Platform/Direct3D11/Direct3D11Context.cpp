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

		//Create device and front/back buffers, also creates swap chain and render context
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, nullptr, &m_pContext);

		//Gain access to texture sub resource in swap chain
		
		wrl::ComPtr<ID3D11Resource> pBackBuffer = nullptr;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer);
		m_pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pRenderTarget);

		//Create depth buffer
		D3D11_DEPTH_STENCIL_DESC ds = {};
		ds.DepthEnable = TRUE;
		ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		ds.DepthFunc = D3D11_COMPARISON_LESS;

		wrl::ComPtr<ID3D11DepthStencilState> pDepthBuffer;
		m_pDevice->CreateDepthStencilState(&ds, &pDepthBuffer);

		m_pContext->OMSetDepthStencilState(pDepthBuffer.Get(), 1u);

		//Create depth texture
		wrl::ComPtr<ID3D11Texture2D> pDepthMap;
		D3D11_TEXTURE2D_DESC td = {};
		td.Width = 1280u;
		td.Height = 720u;
		td.MipLevels = 1u;
		td.ArraySize = 1u;
		td.Format = DXGI_FORMAT_D32_FLOAT;
		td.SampleDesc.Count = 1u;
		td.SampleDesc.Quality = 0u;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		m_pDevice->CreateTexture2D(&td, nullptr, &pDepthMap);
	
		//Create depth view
		D3D11_DEPTH_STENCIL_VIEW_DESC dd = {};
		dd.Format = DXGI_FORMAT_D32_FLOAT;
		dd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dd.Texture2D.MipSlice = 0u;
		m_pDevice->CreateDepthStencilView(pDepthMap.Get(), &dd, &m_pDepthView);

		//Bind depth map
		m_pContext->OMSetRenderTargets(1u, m_pRenderTarget.GetAddressOf(), m_pDepthView.Get());
	}

	void Direct3D11Context::SwapBuffers()
	{
		m_pSwapChain->Present(1u, 0u);
	}
}