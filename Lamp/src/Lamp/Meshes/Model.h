#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Material.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Utility/Structs.h"

namespace Lamp
{
	class Model
	{
	public:
		Model(std::vector<Ref<Mesh>> meshes, Material mat, const std::string& name)
			: m_ModelMatrix(1.f), m_Material(mat), m_Meshes(meshes), m_Name(name)
		{
			float xMax = FLT_MIN, yMax = FLT_MIN, zMax = FLT_MIN;
			float xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX;

			for (auto& vert : meshes[0]->GetVertices())
			{
				//X-axis
				if (vert.position.x < xMin)
				{
					xMin = vert.position.x;
				}
				if (vert.position.x > xMax)
				{
					xMax = vert.position.x;
				}

				//Y-axis
				if (vert.position.y < yMin)
				{
					yMin = vert.position.y;
				}
				if (vert.position.y > yMax)
				{
					yMax = vert.position.y;
				}

				//Z-axis
				if (vert.position.z < zMin)
				{
					zMin = vert.position.z;
				}
				if (vert.position.z > zMax)
				{
					zMax = vert.position.z;
				}
			}

			m_BoundingBox.Max = glm::vec3(xMax, yMax, zMax);
			m_BoundingBox.Min = glm::vec3(xMin, yMin, zMin);
		}

		void Render();

		//Setting
		inline void SetModelMatrix(const glm::mat4& mat) { m_ModelMatrix = mat; }
		inline void SetLGFPath(const std::string& path) { m_LGFPath = path; }

		//Getting
		inline Material& GetMaterial() { return m_Material; }
		inline const std::string& GetName() { return m_Name; }
		inline std::vector<Ref<Mesh>>& GetMeshes() { return m_Meshes; }
		inline std::string& GetLGFPath() { return m_LGFPath; }
		inline const glm::mat4& GetMatrix() { return m_ModelMatrix; }
		inline const SAABB& GetBoundingBox() { return m_BoundingBox; }

	private:
		
	private:
		Material m_Material;
		std::vector<Ref<Mesh>> m_Meshes;
		std::string m_Name;
		std::string m_LGFPath;

		glm::mat4 m_ModelMatrix;
		SAABB m_BoundingBox;
	};
}