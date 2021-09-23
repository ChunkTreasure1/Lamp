#pragma once

#include <vector>
#include "Material.h"

namespace Lamp
{
	class MaterialLibrary
	{
	public:
		static void AddMaterial(const Material& mat);
		static void AddMaterial(const std::filesystem::path& path);
		static void SaveMaterial(const std::string& path, Material& mat);
		static void LoadMaterials();

		static Material& GetMaterial(const std::string& name);
		static std::vector<Material>& GetMaterials() { return m_Materials; }
		static bool IsMaterialLoaded(const std::string& name);

	private:
		static void LoadMaterial(const std::filesystem::path& path);

	private:
		MaterialLibrary() = delete;

		static std::vector<Material> m_Materials;
	};
}