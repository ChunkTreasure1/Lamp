#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Material.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
	class Model
	{
	public:
		Model(std::vector<Ref<Mesh>> meshes, Material mat, const std::string& name)
			: m_ModelMatrix(1.f), m_Material(mat), m_Meshes(meshes), m_Name(name)
		{
		}

		void Render();

		//Setting
		inline void SetModelMatrix(const glm::mat4& mat) { m_ModelMatrix = mat; }
		inline void SetLGFPath(const std::string& path) { m_LGFPath = path; }

		//Getting
		inline Material& GetMaterial() { return m_Material; }
		inline const std::string& GetName() { return m_Name; }
		inline std::vector<Ref<Mesh>>& GetMeshes() { return m_Meshes; }
		inline const std::string& GetLGFPath() { return m_LGFPath; }
		inline const glm::mat4& GetMatrix() { return m_ModelMatrix; }

	private:
		
	private:
		Material m_Material;
		std::vector<Ref<Mesh>> m_Meshes;
		std::string m_Name;
		std::string m_LGFPath;

		glm::mat4 m_ModelMatrix;
	};
}