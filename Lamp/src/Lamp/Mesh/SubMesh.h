#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Lamp/Rendering/Buffers/VertexArray.h"
#include "Lamp/Core/Core.h"

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

	private:
		void SetupMesh();

	private:
		std::vector<uint32_t> m_Indices;
		std::vector<Vertex> m_Vertices;

		Ref<VertexArray> m_pVertexArray;
		uint32_t m_MaterialIndex;

		friend class Renderer3D;
	};
}