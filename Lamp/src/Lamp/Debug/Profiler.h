#pragma once

#include "Lamp/Core/Core.h"

#include <optick.h>

#if LP_ENABLE_PROFILING
#define LP_PROFILE_FRAME(...) OPTICK_FRAME(__VA_ARGS__)
#define LP_PROFILE_FUNCTION(...)  OPTICK_EVENT(__VA_ARGS__)
#define LP_PROFILE_TAG(NAME, ...) OPTICK_TAG(NAME, __VA_ARGS__)
#define LP_PROFILE_SCOPE(NAME) OPTICK_EVENT_DYNAMIC(NAME)
#define LP_PROFILE_THREAD(...) OPTICK_THREAD(__VA_ARGS__)
#else
#define LP_PROFILE_FRAME(...)
#define LP_PROFILE_FUNCTION(...)
#define LP_PROFILE_TAG(NAME, ...)
#define LP_PROFILE_SCOPE(NAME)
#define LP_PROFILE_THREAD(...)
#endif