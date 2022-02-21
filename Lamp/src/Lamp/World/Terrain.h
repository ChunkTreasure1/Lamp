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
		Terrain(Ref<Texture2D> heightMap);
		~Terrain();

		inline const glm::mat4& GetTransform() const { return m_transform; }
		inline Ref<Texture2D> GetHeightMap() { return m_heightMap; }
		inline bool IsValid() const { return m_isValid; }

		void Draw(Ref<RenderPipeline> pipeline);

		static Ref<Terrain> Create(const std::filesystem::path& aHeightMap);
		static Ref<Terrain> Create(Ref<Texture2D> heightMap);

	private:
		void SetupDescriptors(Ref<RenderPipeline> pipeline);
		void GenerateMeshFromHeightMap();
		float GetHeight(uint32_t x, uint32_t y);

		Ref<SubMesh> m_mesh;
		Ref<Texture2D> m_heightMap;

		bool m_isValid = true;
		uint32_t m_scale = 1;
		glm::mat4 m_transform;
		
		VulkanShader::ShaderMaterialDescriptorSet m_descriptorSet;
	};
}