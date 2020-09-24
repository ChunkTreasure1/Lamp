#include "lppch.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Lamp
{
	Ref<spdlog::logger> Log::s_ClientLogger;
	
#ifdef LP_CLIENT
	Ref<spdlog::logger> Log::s_CoreLogger;
#else
	Ref<CoreLogger> Log::s_CoreLogger;
#endif

	void Log::Initialize()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);

#ifdef LP_CLIENT
		s_CoreLogger = spdlog::stdout_color_mt("LAMP");
		s_CoreLogger->set_level(spdlog::level::trace);
#else
		s_CoreLogger = CreateRef<CoreLogger>();
#endif
	}
}