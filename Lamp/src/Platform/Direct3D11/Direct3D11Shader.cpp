#include "lppch.h"
#include "Direct3D11Shader.h"

#include <d3dcompiler.h>
#include <locale>
#include <codecvt>

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Direct3D11/Direct3D11Context.h"
#include "Lamp/Core/Application.h"
#include "Direct3D11DebugLayer.h"

#include "Lamp/Math/Matrix.h"

namespace Lamp
{
	Direct3D11Shader::Direct3D11Shader(std::initializer_list<std::string> paths)
		: m_Paths(paths)
	{
		if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
		{
			if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				std::wstring vertex = converter.from_bytes(m_Paths[0]);
				std::wstring pixel = converter.from_bytes(m_Paths[1]);

				namespace wrl = Microsoft::WRL;
				wrl::ComPtr<ID3DBlob> byteCode = nullptr;
				wrl::ComPtr<ID3DBlob> error = nullptr;

				HRESULT hr = D3DCompileFromFile(L"testShaders\\PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &m_pBlob, &error);
				if (error)
				{
					LP_CORE_CRITICAL((char*)error->GetBufferPointer());
					error->Release();
				}

				hr = D3DCompileFromFile(L"testShaders\\VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &m_pVertexBlob, &error);
				if (error)
				{
					LP_CORE_CRITICAL((char*)error->GetBufferPointer());
					error->Release();
				}

				pContext->GetDevice()->CreatePixelShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &m_pPixel);
				pContext->GetDeviceContext()->PSSetShader(m_pPixel.Get(), nullptr, 0u);

				pContext->GetDevice()->CreateVertexShader(m_pVertexBlob->GetBufferPointer(), m_pVertexBlob->GetBufferSize(), nullptr, &m_pVertex);
				pContext->GetDeviceContext()->VSSetShader(m_pVertex.Get(), nullptr, 0u);
			}
		}
	}

	Direct3D11Shader::~Direct3D11Shader()
	{
	}

	void Direct3D11Shader::Bind() const
	{
	}

	void Direct3D11Shader::Unbind() const
	{
	}

	void Direct3D11Shader::UploadData(const ShaderData& data)
	{
		namespace wrl = Microsoft::WRL;
		namespace dx = DirectX;

		static float angle = 0.f;

		angle++;

		wrl::ComPtr<ID3D11Buffer> pBuf;

		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0u;

		uint32_t size = 0;
		for (auto& uniform : data.Data)
		{
			size += uniform.Size;
		}

		void* base = malloc(size);
		void* ptr = base;
		for (auto& uniform : data.Data)
		{
			uint32_t s = uniform.Size;
			memmove(ptr, uniform.Data, s);
			ptr = static_cast<char*>(ptr) + s;
		}

		bd.ByteWidth = size;
		bd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = base;

		if (!pBuf.Get())
		{
			if (WindowsWindow* pWindow = static_cast<WindowsWindow*>(&Application::Get().GetWindow()))
			{
				if (Direct3D11Context* pContext = static_cast<Direct3D11Context*>(pWindow->GetGraphicsContext().get()))
				{
					pContext->GetDevice()->CreateBuffer(&bd, &sd, &pBuf);

					pContext->GetDeviceContext()->VSSetConstantBuffers(0u, 1u, pBuf.GetAddressOf());
				}
			}
		}
	}
}