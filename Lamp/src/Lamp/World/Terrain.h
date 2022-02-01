#pragma once

#include "Platform/Vulkan/VulkanShader.h"

namespace Lamp
{
	class VertexBuffer;
	class IndexBuffer;
	class Framebuffer;
	class Texture2D;
	class SubMesh;

	class Terrain
	{
	public:
		Terrain(const std::filesystem::path& aHeightMap);
		~Terrain();

		inline const glm::mat4& GetTransform() const { return m_transform; }
		inline Ref<Texture2D> GetHeightMap() { return m_heightMap; }
		inline Ref<RenderPipeline> GetPipeline() { return m_pipeline; }
		inline bool IsValid() const { return m_isValid; }

		Ref<RenderPipeline> SetupRenderPipeline(Ref<Framebuffer> framebuffer);
		void Draw();

		static Ref<Terrain> Create(const std::filesystem::path& aHeightMap);

	private:
		void SetupDescriptors();

		Ref<SubMesh> m_mesh;
		bool m_isValid = true;

		Ref<RenderPipeline> m_pipeline;
		Ref<Texture2D> m_heightMap;

		glm::mat4 m_transform;
		VulkanShader::ShaderMaterialDescriptorSet m_descriptorSet;
	};
}