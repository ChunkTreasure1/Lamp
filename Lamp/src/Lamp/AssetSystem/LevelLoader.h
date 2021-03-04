#pragma once

#include <string>
#include "Lamp/Level/Level.h"

namespace Lamp
{
	class LevelLoader
	{
	public:
		static void LoadLevel(LevelLoadData& data, const std::string& path);

	private:
	};
}