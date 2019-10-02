#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Lamp
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

	//Core logging macros
#define LP_CORE_TRACE(...)     ::Lamp::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LP_CORE_INFO(...)      ::Lamp::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LP_CORE_WARN(...)      ::Lamp::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LP_CORE_ERROR(...)     ::Lamp::Log::GetCoreLogger()->error(__VA_ARGS__)

//Client logging macros
#define LP_TRACE(...)   ::Lamp::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LP_INFO(...)    ::Lamp::Log::GetClientLogger()->info(__VA_ARGS__)
#define LP_WARN(...)    ::Lamp::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LP_ERROR(...)   ::Lamp::Log::GetClientLogger()->error(__VA_ARGS__)
}