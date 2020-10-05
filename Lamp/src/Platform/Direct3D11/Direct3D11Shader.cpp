#include "lppch.h"
#include "Direct3D11Shader.h"

#include <d3dcompiler.h>
#include <locale>
#include <codecvt>

#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Direct3D11/Direct3D11Context.h"
#include "Lamp/Core/Application.h"

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

	void Direct3D11Shader::UploadBool(const std::string& name, bool value) const
	{
	}

	void Direct3D11Shader::UploadInt(const std::string& name, int value) const
	{
	}

	void Direct3D11Shader::UploadFloat(const std::string& name, float value) const
	{
	}

	void Direct3D11Shader::UploadFloat3(const std::string& name, const glm::vec3& value) const
	{
	}

	void Direct3D11Shader::UploadFloat4(const std::string& name, const glm::vec4& value) const
	{
	}

	void Direct3D11Shader::UploadMat4(const std::string& name, const glm::mat4& mat)
	{
	}

	void Direct3D11Shader::UploadMat3(const std::string& name, const glm::mat3& mat)
	{
	}

	void Direct3D11Shader::UploadIntArray(const std::string& name, int* values, uint32_t count) const
	{
	}
}