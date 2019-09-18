#pragma once

#ifdef LP_ENABLE_ASSERTS
	#define LP_ASSERT(x, ...) { if(!(x)) { CC_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define LP_CORE_ASSERT(x, ...) { if(!(x)) { CC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define LP_ASSERT(x, ...)
	#define LP_CORE_ASSERT(x, ...)
#endif