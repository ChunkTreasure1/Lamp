#pragma once

#include "Lamp/Core/Core.h"
#include "Lamp/Objects/BoundingVolume.h"
#include "Lamp/Rendering/Vertex.h"

#include <glm/glm.hpp>
#include <vector>

namespace Lamp
{
	class VertexBuffer;
	class IndexBuffer;
	class SubMesh
	{
	public:
		SubMesh() = default;
		SubMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex);

		inline const Ref<VertexBuffer> GetVertexBuffer() const { return m_vertexBuffer; }
		inline const Ref<IndexBuffer> GetIndexBuffer() const { return m_indexBuffer; }

		inline const uint32_t GetMaterialIndex() { return m_materialIndex; }

		inline std::vector<uint32_t>& GetIndices() { return m_indices; }
		inline std::vector<Vertex>& GetVertices() { return m_vertices; }

		inline const BoundingVolume& GetBoundingVolume() { return m_boundingSphere; }

		static Ref<SubMesh> CreateCube();
		static Ref<SubMesh> CreateQuad();
		static Ref<SubMesh> CreateSphere();

	private:
		friend class Renderer3D;

		void SetupMesh();

		std::vector<uint32_t> m_indices;
		std::vector<Vertex> m_vertices;

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;

		uint32_t m_materialIndex;

		BoundingSphere m_boundingSphere;
	};
}