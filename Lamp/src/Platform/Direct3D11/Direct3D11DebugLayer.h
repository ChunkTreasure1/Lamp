#pragma once

#include <exception>
#include <string>

#define THROW_FAILED(hrcall) if(FAILED(hr = (hrcall))) throw Lamp::HrException(__LINE__, __FILE__, hr);
#define DEVICE_REMOVED_EXCEPTION(hr) Lamp::DeviceRemovedException(__LINE__, __FILE__, (hr))

namespace Lamp
{
	class LampException : public std::exception
	{
	public:
		LampException(int line, const char* file) noexcept;
		virtual const char* what() const noexcept override;
		virtual const char* GetType() const noexcept;

		int GetLine() const noexcept;
		const std::string& GetFile() const noexcept;
		std::string GetOriginString() const noexcept;

	private:
		int m_Line;
		std::string m_File;

	protected:
		mutable std::string m_WhatBuffer;
	};

	class Exception : public LampException
	{
		using LampException::LampException;
	};

	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;

		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;

	private:
		HRESULT m_HR;
	};

	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	};
}