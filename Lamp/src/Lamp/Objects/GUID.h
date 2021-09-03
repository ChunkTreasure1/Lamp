#pragma once

#include <string>

struct GUID
{
	GUID(const std::string& g)
		: guid(g)
	{}

	std::string guid;
};