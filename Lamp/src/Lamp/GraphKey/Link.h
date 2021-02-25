#pragma once

#include <functional>


namespace Lamp
{
	struct Attribute;

	struct Link
	{
		Attribute* output = nullptr;
		Attribute* input = nullptr;

		uint32_t id;
	};
}