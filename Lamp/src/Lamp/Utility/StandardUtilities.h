#pragma once

namespace Utility
{
	template<class Container, typename T>
	static bool RemoveFromContainer(Container& container, T& item)
	{
		if (auto it = std::find(container.begin(), container.end(), item); it != container.end())
		{
			container.erase(it);
			return true;
		}

		return false;
	}

	template<class Container, class pred>
	static bool RemoveFromContainerIf(Container& container, pred predicate)
	{
		if (auto it = std::find_if(container.begin(), container.end(), predicate); it != container.end())
		{
			container.erase(it);
			return true;
		}

		return false;
	}

	template<class Container, typename T>
	static size_t GetIndexInContainer(const Container& container, const T& item)
	{
		if (auto it = std::find(container.begin(), container.end(), item); it != container.end())
		{
			return std::distance(container.begin(), it);
		}

		return -1;
	}

	template<typename T>
	static bool CompareAny(const std::any& first, const std::any& second)
	{
		return std::any_cast<T>(first) == std::any_cast<T>(second);
	}
}