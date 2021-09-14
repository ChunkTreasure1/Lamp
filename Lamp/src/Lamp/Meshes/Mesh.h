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

	struct SubMeshData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t matId;
	};

	struct MeshData
	{
		std::vector<Ref<SubMesh>> meshes;
		std::map<uint32_t, Material> materials;
		std::string name;
		AABB boundingBox;
		std::string path;
	};

	struct MeshLoadData
	{
		std::vector<MeshData> meshes;
		std::map<uint32_t, Material> materials;
		std::string name;
		AABB boundingBox;
		std::string path;
	};

	class Mesh : public Asset
	{
	public:
		Mesh(const std::string& name, std::vector<SubMeshData> meshes, std::map<uint32_t, Material> mats, AABB bb)
			: m_Materials(mats), m_Name(name), m_BoundingBox(bb)
		{
			for (auto& mesh : meshes)
			{
				m_Meshes.push_back(CreateRef<SubMesh>(mesh.vertices, mesh.indices, mesh.matId));
			}
		}

		Mesh()
		{}

		friend class ResourceCache;

		void Render(size_t id = -1, const glm::mat4& transform = glm::mat4(1.f), bool forward = false);
		void RenderBoundingBox(const glm::mat4& transform);

		//Setting
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetMaterial(const Material& mat, uint32_t id) 
		{
			if (m_Materials.find(id) == m_Materials.end())
			{
				LP_CORE_ASSERT(false, "Mesh does not contain the specified material id!");
				return;
			}
			m_Materials[id] = mat; 
		}

		//Getting
		inline Material& GetMaterial(uint32_t id) 
		{
			if (m_Materials.find(id) == m_Materials.end())
			{
				LP_CORE_ASSERT(false, "Mesh does not contain the specified material id!");
			}
			return m_Materials[id]; 
		}
		inline std::map<uint32_t, Material>& GetMaterials() { return m_Materials; }
		inline const std::string& GetName() { return m_Name; }
		inline std::vector<Ref<SubMesh>>& GetSubMeshes() { return m_Meshes; }
		inline AABB& GetBoundingBox() { return m_BoundingBox; }

		static AssetType GetStaticType() { return AssetType::Mesh; }
		virtual AssetType GetType() override { return GetStaticType(); }
		
	private:
		std::vector<Ref<SubMesh>> m_Meshes;
		std::map<uint32_t, Material> m_Materials;
		std::string m_Name;

		AABB m_BoundingBox;
	};
}