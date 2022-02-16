#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Lamp/Rendering/Buffers/VertexArray.h"
#include "Lamp/Core/Core.h"

#include "Lamp/Objects/BoundingVolume.h"

namespace Lamp
{
	class SubMesh
	{
	public:
		SubMesh() = default;
		SubMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex);

		inline const Ref<VertexArray>& GetVertexArray() { return m_pVertexArray; }
		inline const uint32_t GetMaterialIndex() { return m_MaterialIndex; }

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

		Ref<VertexArray> m_pVertexArray;
		uint32_t m_MaterialIndex;

		BoundingSphere m_boundingSphere;

		friend class Renderer3D;
	};
}