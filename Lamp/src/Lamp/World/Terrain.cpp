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
		const float yScale = 64.f / 256.f; //TODO: move to some form of settings
		const float yShift = 16.f;		   //TODO: move to some form of settings

		for (uint32_t i = 0; i < height; i++)
		{
			for (uint32_t j = 0; j < width; j++)
			{
				uint8_t* texel = data + (j + width * i) * channels;
				uint8_t y = texel[0];

				vertices.emplace_back(glm::vec3{ -height / 2.f + i, (int)y * yScale - yShift, -width / 2.f + j });
			}
		}

		stbi_image_free(data);

		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < height - 1; i++)
		{
			for (uint32_t j = 0; j < width; j++)
			{
				for (uint32_t k = 0; k < 2; k++)
				{
					indices.emplace_back(j + width * (i + k));
				}
			}
		}

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(Vertex) * vertices.size());
		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, indices.size());

		Ref<VertexArray> vertexArray = VertexArray::Create();
		vertexArray->AddVertexBuffer(vertexBuffer);
		vertexArray->SetIndexBuffer(indexBuffer);

		std::vector<Ref<SubMesh>> subMesh;
		subMesh.push_back(CreateRef<SubMesh>(vertices, indices, 0));

		std::map<uint32_t, Ref<Material>> materials;
		materials[0] = MaterialLibrary::GetMaterial("base");

		m_terrainMesh = CreateRef<Mesh>("Terrain", subMesh, materials, AABB());
	}
}