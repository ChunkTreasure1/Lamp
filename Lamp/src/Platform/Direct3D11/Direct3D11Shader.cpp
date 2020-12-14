#include "lppch.h"
#include "Direct3D11Shader.h"

#include <d3dcompiler.h>
#include <locale>
#include <codecvt>

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Direct3D11/Direct3D11Context.h"
#include "Lamp/Core/Application.h"
#include "Direct3D11DebugLayer.h"

#include <DirectXMath.h>

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

				D3DReadFileToBlob(pixel.c_str(), &m_pBlob);
				pContext->GetDevice()->CreatePixelShader(m_pBlob->GetBufferPointer(), m_pBlob->GetBufferSize(), nullptr, &m_pPixel);
				pContext->GetDeviceContext()->PSSetShader(m_pPixel.Get(), nullptr, 0u);

				D3DReadFileToBlob(vertex.c_str(), &m_pVertexBlob);
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

		uint32_t typeCounts[9];
		for (int i = 0; i < 9; i++)
		{
			typeCounts[i] = 0;
		}

		//for (auto& uniform : data.Data)
		//{
		//	typeCounts[uniform.Type]++;
		//}

		typeCounts[7] = 2;

		static float angle = 0.f;

		ShaderBuffer b;
		AllocateUniforms(b, typeCounts);

		b.mat4s[0] = Math::buildMatrix({ 0.f, 0.f, 4.f }, { 1.f, 1.f, 1.f }, { glm::radians(angle), 0.f, glm::radians(angle) });
		b.mat4s[1] = Math::perspective(glm::radians(45.f), 16.f / 9.f, 0.1f, 100.f);

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

		bd.ByteWidth = sizeof(b);
		bd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = &b;

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

	void Direct3D11Shader::AllocateUniforms(ShaderBuffer& b, const uint32_t* values)
	{
		if (values[0] > 0) b.bools = new bool[values[0]];
		if (values[1] > 0) b.ints = new int[values[1]];
		if (values[2] > 0) b.floats = new float[values[2]];
		if (values[3] > 0) b.vec2s = new glm::vec2[values[3]];
		if (values[4] > 0) b.vec3s = new glm::vec3[values[4]];
		if (values[5] > 0) b.vec4s = new glm::vec4[values[5]];
		if (values[7] > 0) b.mat4s = new Math::mat4[values[7]];
	}
}