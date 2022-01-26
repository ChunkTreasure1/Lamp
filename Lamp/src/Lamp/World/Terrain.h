#pragma once

#include "Platform/Vulkan/VulkanShader.h"

namespace Lamp
{
	class VertexBuffer;
	class IndexBuffer;
	class Framebuffer;
	class Texture2D;

	class Terrain
	{
	public:
		Terrain(const std::filesystem::path& aHeightMap);
		//inline Ref<Mesh> GetMesh() const { return m_terrainMesh; }

		inline const glm::mat4& GetTransform() const { return m_transform; }
		inline Ref<VertexBuffer> GetVertexBuffer() { return m_vertexBuffer; }
		inline Ref<IndexBuffer> GetIndexBuffer() { return m_indexBuffer; }

		inline Ref<Texture2D> GetHeightMap() { return m_heightMap; }

		void Draw();

	private:
		void SetupRenderPass(Ref<Framebuffer> framebuffer);
		void SetupDescriptors();

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;
		Ref<RenderPipeline> m_pipeline;
		Ref<Texture2D> m_heightMap;

		glm::mat4 m_transform;

		std::vector<VulkanShader::ShaderMaterialDescriptorSet> m_descriptorSets;
	};
}