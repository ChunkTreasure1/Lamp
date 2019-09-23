#pragma once
#include <utility>

#define BIT(x) (1 << x)

#define LP_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)