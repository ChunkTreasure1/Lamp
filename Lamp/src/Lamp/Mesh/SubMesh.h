#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Lamp/Core/Core.h"

#include "Lamp/Objects/BoundingVolume.h"
#include "Lamp/Rendering/Vertex.h"

namespace Lamp
{
	class VertexBuffer;
	class IndexBuffer;
	class SubMesh
	{
	public:
		SubMesh() = default;
		SubMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex);

		inline const uint32_t GetMaterialIndex() { return m_MaterialIndex; }

		inline const Ref<VertexBuffer> GetVertexBuffer() const { return m_vertexBuffer; }
		inline const Ref<IndexBuffer> GetIndexBuffer() const { return m_indexBuffer; }

		inline std::vector<uint32_t>& GetIndices() { return m_Indices; }
		inline std::vector<Vertex>& GetVertices() { return m_Vertices; }

		inline const BoundingVolume& GetBoundingVolume() { return m_boundingSphere; }

		static Ref<SubMesh> CreateCube();
		static Ref<SubMesh> CreateQuad();
		static Ref<SubMesh> CreateSphere();

	private:
		void SetupMesh();

		std::vector<uint32_t> m_Indices;
		std::vector<Vertex> m_Vertices;

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;

		uint32_t m_MaterialIndex;

		BoundingSphere m_boundingSphere;

		friend class Renderer3D;
	};
}