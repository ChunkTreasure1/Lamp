#pragma once

#include <string>

#include "SubMesh.h"
#include "Materials/Material.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	struct AABB
	{
		glm::vec3 Max = glm::vec3(0.f);
		glm::vec3 Min = glm::vec3(0.f);

		glm::vec3 StartMax = glm::vec3(0.f);
		glm::vec3 StartMin = glm::vec3(0.f);
	};

	class Mesh : public Asset
	{
	public:
		Mesh(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb);
		Mesh();

		friend class ResourceCache;

		void Render(size_t id = -1, const glm::mat4& transform = glm::mat4(1.f));
		void Load(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb);

		//Setting
		inline void SetName(const std::string& name) { m_name = name; }
		void SetMaterial(Ref<Material> mat, uint32_t id);

		//Getting
		Ref<Material> GetMaterial(uint32_t id);
		inline std::map<uint32_t, Ref<Material>>& GetMaterials() { return m_materials; }
		inline const std::string& GetName() { return m_name; }
		inline std::vector<Ref<SubMesh>>& GetSubMeshes() { return m_meshes; }
		inline AABB& GetBoundingBox() { return m_boundingBox; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Mesh> Create(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb);
		static Ref<Mesh> Create();

	private:
		std::vector<Ref<SubMesh>> m_meshes;
		std::map<uint32_t, Ref<Material>> m_materials;
		std::string m_name;

		AABB m_boundingBox;
	};
}