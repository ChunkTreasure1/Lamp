#pragma once

#include <vector>
#include "Material.h"

namespace Lamp
{
	class MaterialLibrary
	{
	public:
		static void AddMaterial(Ref<Material> mat);
		static void AddMaterial(const std::filesystem::path& path);
		static void LoadMaterials();

		static Ref<Material> GetMaterial(const std::string& name);
		static std::vector<Ref<Material>>& GetMaterials() { return m_Materials; }
		static bool IsMaterialLoaded(const std::string& name);

	private:
		static void LoadMaterial(const std::filesystem::path& path);

	private:
		MaterialLibrary() = delete;

		static std::vector<Ref<Material>> m_Materials;
	};
}