#pragma once

#include <stdint.h>
#include <string>

namespace Lamp
{
	struct ObjectLayer
	{
		ObjectLayer(const std::string& name, uint32_t id, bool active)
			: Name(name), ID(id), Active(active)
		{}

		std::string Name;
		uint32_t ID;
		bool Active;

		std::vector<Object*> Objects;
	};
}