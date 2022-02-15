#pragma once

#include <vector>
#include "Material.h"

namespace Lamp
{
	class MaterialLibrary
	{
	public:
		MaterialLibrary();

		void Initialize();
		void Shutdown();

		void AddMaterial(Ref<Material> mat);
		void AddMaterial(const std::filesystem::path& path);
		void LoadMaterials();

		std::vector<Ref<Material>>& GetMaterials() { return m_materials; }
		bool IsMaterialLoaded(const std::string& name);

		static Ref<Material> GetMaterial(const std::string& name);
		static MaterialLibrary& Get();

	private:
		void LoadMaterial(const std::filesystem::path& path);

		static MaterialLibrary* s_instance;
		std::vector<Ref<Material>> m_materials;
	};
}