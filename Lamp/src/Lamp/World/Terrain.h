#pragma once

namespace Lamp
{
	class VertexBuffer;
	class IndexBuffer;
	class Terrain
	{
	public:
		Terrain(const std::filesystem::path& aHeightMap);

		//inline Ref<Mesh> GetMesh() const { return m_terrainMesh; }

	private:
		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;
	};
}