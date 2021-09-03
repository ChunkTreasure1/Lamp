#pragma once

#include <functional>


namespace Lamp
{
	struct InputAttribute;
	struct OutputAttribute;

	struct Link
	{
		InputAttribute* pInput = nullptr;
		OutputAttribute* pOutput = nullptr;

		uint32_t id;
	};
}