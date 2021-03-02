#pragma once

#include "Model.h"

namespace Lamp
{
	class ModelLoader
	{
	public:
		static void LoadFromFile(ModelLoadData& data, const std::string& path);
		static ModelData GenerateMesh(ModelLoadData& data);
	private:
	};
}