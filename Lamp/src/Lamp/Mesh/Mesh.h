#pragma once

#include <string>

#include "SubMesh.h"
#include "Materials/Material.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class MaterialInstance;
	class Mesh : public Asset
	{
	public:
		Mesh(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats);
		Mesh() = default;

		void Render(const glm::mat4& transform = glm::mat4(1.f), size_t id = -1, std::map<uint32_t, Ref<MaterialInstance>>& materials = std::map<uint32_t, Ref<MaterialInstance>>());

		//Setting
		inline void SetName(const std::string& name) { m_name = name; }

		//Getting
		inline std::map<uint32_t, Ref<Material>>& GetMaterials() { return m_materials; }
		inline const std::string& GetName() { return m_name; }
		inline std::vector<Ref<SubMesh>>& GetSubMeshes() { return m_subMeshes; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetType() override { return GetStaticType(); }

	private:
		friend class ResourceCache;

		std::vector<Ref<SubMesh>> m_subMeshes;
		std::map<uint32_t, Ref<Material>> m_materials;
		std::string m_name;
	};
}