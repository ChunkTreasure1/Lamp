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
		inline const uint32_t GetMaterialIndex() { return m_materialIndex; }
		
		inline const BoundingVolume& GetBoundingVolume() { return m_boundingSphere; }

		static Ref<SubMesh> CreateCube();
		static Ref<SubMesh> CreateQuad();
		static Ref<SubMesh> CreateSphere();

	private:
		void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		BoundingSphere m_boundingSphere;

		Ref<VertexArray> m_pVertexArray;
		uint32_t m_materialIndex;

		friend class Renderer3D;
	};
}