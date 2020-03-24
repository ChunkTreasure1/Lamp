#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Lamp/Rendering/Vertices/VertexArray.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	class Mesh
	{
	public:
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex);

		inline const Ref<VertexArray>& GetVertexArray() { return m_pVertexArray; }

	private:
		void SetupMesh();

	private:
		std::vector<uint32_t> m_Indices;
		std::vector<Vertex> m_Vertices;

		Ref<VertexArray> m_pVertexArray;
		uint32_t m_MaterialIndex;
	};
}