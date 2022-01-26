#include "lppch.h"
#include "Terrain.h"

#include "Lamp/Rendering/Buffers/VertexBuffer.h"
#include "Lamp/Rendering/Buffers/VertexArray.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"

#include "Lamp/Mesh/Materials/MaterialLibrary.h"
#include "Lamp/Mesh/Mesh.h"

#include "Platform/Vulkan/VulkanRenderPipeline.h"

namespace Lamp
{
	Terrain::Terrain(const std::filesystem::path& aHeightMap)
	{
		m_heightMap = Texture2D::Create(aHeightMap);

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		const uint32_t patchSize = 64;
		const float uvScale = 1.f;

		const uint32_t vertexCount = patchSize * patchSize;
		vertices.resize(vertexCount);

		const float wx = 2.f;
		const float wy = 2.f;

		for (uint32_t x = 0; x < patchSize; x++)
		{
			for (uint32_t y = 0; y < patchSize; y++)
			{
				uint32_t index = (x + y * patchSize);
				vertices[index].position = { x * wx + wx / 2.f - (float)patchSize * wx / 2.f, 0.f, y * wy + wy / 2.f - patchSize * wy / 2.f };
				vertices[index].textureCoords = glm::vec2{ (float)x / patchSize, (float)y / patchSize } * uvScale;
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

		m_vertexBuffer = VertexBuffer::Create(vertices, sizeof(Vertex) * vertices.size());
		m_indexBuffer = IndexBuffer::Create(indices, indices.size());
		m_transform = glm::scale(glm::mat4(1.f), glm::vec3{ 2.f, 1.f, 2.f });
	}

	void Terrain::Draw()
	{
	}

	void Terrain::SetupRenderPass(Ref<Framebuffer> framebuffer)
	{
	}

	void Terrain::SetupDescriptors()
	{
	}
}