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
		std::vector<uint32_t> indices;

		const uint32_t patchSize = 64;
		const float uvScale = 1.f;

		const float wx = 2.f;
		const float wy = 2.f;

		for (uint32_t x = 0; x < patchSize; x++)
		{
			for (uint32_t y = 0; y < patchSize; y++)
			{
				vertices.emplace_back(glm::vec3{ x * wx + wx / 2.f - (float)patchSize * wx / 2.f, 0.f, y * wy + wy / 2.f - (float)patchSize * wy / 2.f }, glm::vec2{ (float)x / patchSize, (float)y / patchSize } * uvScale);
			}
		}

		const uint32_t w = patchSize - 1;
		const uint32_t indexCount = w * w * 4;

		indices.resize(indexCount);

		for (uint32_t x = 0; x < w; x++)
		{
			for (uint32_t y = 0; y < w; y++)
			{
				uint32_t index = (x + y * w) * 4;

				indices[index] = (x + y * patchSize);
				indices[index + 1] = (indices[index] + patchSize);
				indices[index + 2] = (indices[index + 1] + 1);
				indices[index + 3] = (indices[index] + 1);
			}
		}

		stbi_image_free(data);

		m_vertexBuffer = VertexBuffer::Create(vertices, sizeof(Vertex) * vertices.size());
		m_indexBuffer = IndexBuffer::Create(indices, indices.size());
	}
}