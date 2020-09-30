#pragma once
#include <utility>
#include "GlobalEnvironment.h"

#define BIT(x) (1 << x)

#define LP_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#ifdef LP_DEBUG
	#if defined(LP_PLATFORM_WINDOWS)
		#define LP_DEBUGBREAK() __debugbreak()
		#define GLFW_EXPOSE_NATIVE_WIN32
	#else
		#error "Platform does not support debugbreak!"
	#endif
#else
	#define LP_DEBUGBREAK()
#endif // DEBUG

#ifdef LP_ENABLE_ASSERTS
	#define LP_ASSERT(x, ...) { if(!(x)) { LP_ERROR("Assertion failed: {0}", __VA_ARGS__); LP_DEBUGBREAK(); } }
	#define LP_CORE_ASSERT(x, ...) { if(!(x)) { LP_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); LP_DEBUGBREAK(); } }
#else
	#define LP_ASSERT()
	#define LP_CORE_ASSERT()
#endif

#define RegisterData(x) static_cast<void*>(x)
#define DefaultBankPath "assets\\audio\\banks"

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

/////Global environment/////
extern GlobalEnvironment* g_pEnv;
////////////////////////////