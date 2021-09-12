#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Materials/Material.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		uint32_t matId;
	};

	struct ModelData
	{
		std::vector<Ref<Mesh>> meshes;
		Material material;
		std::string name;
		SAABB boundingBox;
		std::string path;
	};

	struct ModelLoadData
	{
		std::vector<MeshData> meshes;
		Material material;
		std::string name;
		SAABB boundingBox;
		std::string path;
	};

	class Model
	{
	public:
		Model(std::vector<Ref<Mesh>> meshes, Material mat, const std::string& name, SAABB bb)
			: m_ModelMatrix(1.f), m_Material(mat), m_Meshes(meshes), m_Name(name), m_BoundingBox(bb)
		{
		}

		Model(std::vector<Ref<Mesh>> meshes, Material mat, const std::string& name, SAABB bb, const std::string& path)
			: m_ModelMatrix(1.f), m_Material(mat), m_Meshes(meshes), m_Name(name), m_BoundingBox(bb), m_LGFPath(path)
		{
		}

		Model()
			: m_ModelMatrix(glm::mat4(1.f))
		{}

		friend class ResourceCache;

		void Render(size_t id = -1, bool forward = false);
		void RenderBoundingBox();

		//Setting
		inline void SetModelMatrix(const glm::mat4& mat) { m_ModelMatrix = mat; }
		inline void SetLGFPath(const std::string& path) { m_LGFPath = path; }
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetMaterial(const Material& mat) { m_Material = mat; }
		inline void SetData(ModelData& data)
		{
			m_Meshes = data.meshes;
			m_Material = data.material;
			m_Name = data.name;
			m_BoundingBox = data.boundingBox;
			m_LGFPath = data.path;
		}

		//Getting
		inline Material& GetMaterial() { return m_Material; }
		inline const std::string& GetName() { return m_Name; }
		inline std::vector<Ref<Mesh>>& GetMeshes() { return m_Meshes; }
		inline std::string& GetLGFPath() { return m_LGFPath; }
		inline const glm::mat4& GetMatrix() { return m_ModelMatrix; }
		inline SAABB& GetBoundingBox() { return m_BoundingBox; }
		
	private:
		Material m_Material;
		std::vector<Ref<Mesh>> m_Meshes;
		std::string m_Name;
		std::string m_LGFPath;

		glm::mat4 m_ModelMatrix;
		SAABB m_BoundingBox;
	};
}