#pragma once

#include "Platform/Vulkan/VulkanShader.h"

namespace Lamp
{
	class VertexBuffer;
	class IndexBuffer;
	class Framebuffer;

	class Terrain
	{
	public:
		Terrain(const std::filesystem::path& aHeightMap, Ref<Framebuffer> framebuffer);
		//inline Ref<Mesh> GetMesh() const { return m_terrainMesh; }

		void Draw();

	private:
		void SetupRenderPass(Ref<Framebuffer> framebuffer);
		void SetupDescriptors();

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;
		Ref<RenderPipeline> m_pipeline;

		std::vector<VulkanShader::ShaderMaterialDescriptorSet> m_descriptorSets;
	};
}