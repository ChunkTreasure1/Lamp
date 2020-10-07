#include "lppch.h"
#include "Direct3D11DebugLayer.h"

namespace Lamp
{
	LampException::LampException(int line, const char* file) noexcept
		: m_Line(line), m_File(file)
	{
	}

	const char* LampException::what() const noexcept
	{
		std::ostringstream oss;
		oss << GetType() << std::endl << GetOriginString();

		m_WhatBuffer = oss.str();
		return m_WhatBuffer.c_str();
	}

	const char* LampException::GetType() const noexcept
	{
		return "Lamp Exception";
	}

	int LampException::GetLine() const noexcept
	{
		return m_Line;
	}

	const std::string& LampException::GetFile() const noexcept
	{
		return m_File;
	}

	std::string LampException::GetOriginString() const noexcept
	{
		std::ostringstream oss;
		oss << "[File] " << m_File << std::endl << "[Line] " << m_Line;
		return oss.str();
	}

	HrException::HrException(int line, const char* file, HRESULT hr) noexcept
		: Exception(line, file)
	{
	}

	const char* HrException::what() const noexcept
	{
		return nullptr;
	}

	const char* HrException::GetType() const noexcept
	{
		return nullptr;
	}

	HRESULT HrException::GetErrorCode() const noexcept
	{
		return E_NOTIMPL;
	}

	std::string HrException::GetErrorString() const noexcept
	{
		return std::string();
	}

	std::string HrException::GetErrorDescription() const noexcept
	{
		return std::string();
	}

	const char* DeviceRemovedException::GetType() const noexcept
	{
		return nullptr;
	}
}