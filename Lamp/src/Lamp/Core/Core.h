#pragma once
#include <utility>

#define BIT(x) (1 << x)

#define LP_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Lamp
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
}

namespace Game
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
}