#pragma once

#include "Lamp/Meshes/Model.h"

namespace Lamp
{
	class GeometrySystem
	{
	public:
		Model ImportModel(const std::string& path);
		Model LoadFromFile(const std::string& path);
		bool SaveToPath(const Model& model, const std::string& path);
	};
}