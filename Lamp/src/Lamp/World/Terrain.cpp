#include "lppch.h"
#include "Terrain.h"

#include "Lamp/Core/Application.h"

#include "Lamp/Rendering/Buffers/VertexBuffer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Rendering/Renderer.h"

#include "Lamp/Mesh/Materials/MaterialLibrary.h"
#include "Lamp/Mesh/Mesh.h"

#include "Platform/Vulkan/VulkanDevice.h"

namespace Lamp
{
	Terrain::Terrain(const std::filesystem::path& aHeightMap)
	{
		if (!std::filesystem::exists(aHeightMap))
		{
			LP_CORE_ERROR("[Terrain]: Unable to load file {0}!", aHeightMap.string());
			m_isValid = false;
			return;
		}

		m_heightMap = Texture2D::Create(aHeightMap);
	
		GenerateMeshFromHeightMap();
	}

	Terrain::Terrain(Ref<Texture2D> heightMap)
	{
		m_heightMap = heightMap;
		GenerateMeshFromHeightMap();
	}

	Terrain::~Terrain()
	{
	}

	void Terrain::Draw(Ref<RenderPipeline> pipeline)
	{
		SetupDescriptors(pipeline);
		RenderCommand::SubmitMesh(m_mesh, nullptr, m_descriptorSet.descriptorSets, (void*)glm::value_ptr(m_transform));
	}

	Ref<Terrain> Terrain::Create(const std::filesystem::path& heightMap)
	{
		return CreateRef<Terrain>(heightMap);
	}

	Ref<Terrain> Terrain::Create(Ref<Texture2D> heightMap)
	{
		return CreateRef<Terrain>(heightMap);
	}

	void Terrain::SetupDescriptors(Ref<RenderPipeline> pipeline)
	{
		auto vulkanShader = pipeline->GetSpecification().shader;
		auto device = VulkanContext::GetCurrentDevice();
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		auto descriptorSet = vulkanShader->CreateDescriptorSets();
		std::vector<VkWriteDescriptorSet> writeDescriptors;

		auto vulkanUniformBuffer = pipeline->GetSpecification().uniformBufferSets->Get(0, 0, currentFrame);
		auto vulkanTerrainBuffer = Renderer::Get().GetStorage().terrainDataBuffer;

		writeDescriptors.emplace_back(*vulkanShader->GetDescriptorSet("CameraDataBuffer"));
		writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
		writeDescriptors[0].pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

		writeDescriptors.emplace_back(*vulkanShader->GetDescriptorSet("DirectionalLightBuffer"));
		writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
		writeDescriptors[1].pBufferInfo = &vulkanTerrainBuffer->GetDescriptorInfo();

		if (m_heightMap)
		{
			writeDescriptors.emplace_back(*vulkanShader->GetDescriptorSet("u_HeightMap"));
			writeDescriptors[2].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[2].pImageInfo = &m_heightMap->GetDescriptorInfo();
		}

		auto vulkanScreenUB = pipeline->GetSpecification().uniformBufferSets->Get(3, 0, currentFrame);
		writeDescriptors.emplace_back(*vulkanShader->GetDescriptorSet("ScreenDataBuffer"));
		writeDescriptors[3].dstSet = descriptorSet.descriptorSets[0];
		writeDescriptors[3].pBufferInfo = &vulkanScreenUB->GetDescriptorInfo();

		vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		m_descriptorSet = descriptorSet;
	}

	void Terrain::GenerateMeshFromHeightMap()
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		const uint32_t patchSize = 64;
		const float uvScale = 1.f;

		m_scale = m_heightMap->GetHeight() / patchSize;

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
				vertices[index].textureCoords = glm::vec2{ (float)x / patchSize, (float)y / patchSize } *uvScale;
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

		for (uint32_t x = 0; x < patchSize; x++)
		{
			for (uint32_t y = 0; y < patchSize; y++)
			{
				float heights[3][3];
				for (int32_t hx = -1; hx <= 1; hx++)
				{
					for (int32_t hy = -1; hy <= 1; hy++)
					{
						heights[hx + 1][hy + 1] = GetHeight(x + hx, y + hy);
					}
				}

				glm::vec3 normal;
				normal.x = heights[0][0] - heights[2][0] + 2.f * heights[0][1] - 2.f * heights[2][1] + heights[0][2] - heights[2][2];
				normal.y = heights[0][0] + 2.f * heights[1][0] + heights[2][0] - heights[0][2] - 2.f * heights[1][2] - heights[2][2];
			
				normal.y = 0.25f * sqrtf(1.f - normal.x * normal.x - normal.y * normal.y);
				vertices[x + y * patchSize].normal = glm::normalize(normal * glm::vec3(2.f, 1.f, 2.f));
			}
		}

		m_mesh = CreateRef<SubMesh>(vertices, indices, 0);
		m_transform = glm::scale(glm::mat4(1.f), glm::vec3{ 2.f, 1.f, 2.f });
	}

	float Terrain::GetHeight(uint32_t x, uint32_t y)
	{
		glm::ivec2 pos = glm::ivec2{ x, y } * glm::ivec2{ m_scale, m_scale };
		pos.x = std::max(0, std::min(pos.x, (int)m_heightMap->GetWidth() - 1));
		pos.y = std::max(0, std::min(pos.y, (int)m_heightMap->GetWidth() - 1));
		pos /= glm::ivec2(m_scale);
		
		auto buffer = m_heightMap->GetData();

		return buffer.Read<uint16_t>((pos.x + pos.y * m_heightMap->GetHeight()) * m_scale) / 65535.f;
	}
}