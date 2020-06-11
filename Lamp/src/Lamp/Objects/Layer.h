#pragma once

namespace Lamp
{
	class ObjectLayer
	{
		public:
		ObjectLayer(uint32_t num, const std::string& name, bool state)
			: Number(num), Name(name), IsDestroyable(state)
		{}

		ObjectLayer(uint32_t num)
			: Number(num), Name("Layer"), IsDestroyable(true)
		{}

		ObjectLayer()
			: Number(0), Name("Layer"), IsDestroyable(false)
		{}

		uint32_t Number;
		std::string Name;
		bool IsDestroyable;
	};
}