#include "lppch.h"
#include "Terrain.h"

#include "Lamp/Rendering/Buffers/VertexBuffer.h"
#include "Lamp/Rendering/Buffers/VertexArray.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"
#include "Lamp/Mesh/Mesh.h"

//TODO: remove
#include <stb/stb_image.h>

namespace Lamp
{
	Terrain::Terrain(const std::filesystem::path& aHeightMap)
	{
		int width, height, channels;
		uint8_t* data = stbi_load(aHeightMap.string().c_str(), &width, &height, &channels, 0);

		std::vector<Vertex> vertices;

		const uint32_t resolution = 20;
		for (uint32_t i = 0; i <= resolution - 1; i++)
		{
			for (uint32_t j = 0; j <= resolution - 1; j++)
			{
				vertices.emplace_back(glm::vec3{ -width / 2.f + width * i / resolution, 0.f, -height / 2.f + height * j / resolution }, glm::vec2{ i / (float)resolution, j / (float)resolution });
				vertices.emplace_back(glm::vec3{ -width / 2.f + width * (i + 1) / resolution, 0.f, -height / 2.f + height * j / resolution }, glm::vec2{ (i + 1) / (float)resolution, j / (float)resolution });
				vertices.emplace_back(glm::vec3{ -width / 2.f + width * i / resolution, 0.f, -height / 2.f + height * (j + 1) / resolution }, glm::vec2{ i / (float)resolution, (j + 1) / (float)resolution });
				vertices.emplace_back(glm::vec3{ -width / 2.f + width * (i + 1) / resolution, 0.f, -height / 2.f + height * (j + 1) / resolution }, glm::vec2{ (i + 1) / (float)resolution, (j + 1) / (float)resolution });
			}
		}   

		stbi_image_free(data);

		std::vector<Ref<SubMesh>> subMesh;
		subMesh.push_back(CreateRef<SubMesh>(vertices, std::vector<uint32_t>(), 0));

		std::map<uint32_t, Ref<Material>> materials;
		materials[0] = MaterialLibrary::GetMaterial("base");

		m_terrainMesh = CreateRef<Mesh>("Terrain", subMesh, materials, AABB());
	}
}