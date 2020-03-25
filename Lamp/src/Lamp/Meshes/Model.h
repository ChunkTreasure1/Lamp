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
		Model(std::vector<Mesh> meshes, Material mat, const std::string& name)
			: m_ModelMatrix(1.f), m_Material(mat), m_Meshes(meshes), m_Name(name)
		{
		}

		void Draw();

		//Setting
		inline void SetModelMatrix(const glm::mat4& mat) { m_ModelMatrix = mat; }

		//Getting
		inline Material& GetMaterial() { return m_Material; }

	private:
		
	private:
		Material m_Material;
		std::vector<Mesh> m_Meshes;
		std::string m_Name;

		glm::mat4 m_ModelMatrix;
	};
}