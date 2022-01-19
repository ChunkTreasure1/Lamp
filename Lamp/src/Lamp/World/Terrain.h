#pragma once

namespace Lamp
{
	class Mesh;
	class Terrain
	{
	public:
		Terrain(const std::filesystem::path& aHeightMap);

		inline Ref<Mesh> GetMesh() const { return m_terrainMesh; }

	private:
		Ref<Mesh> m_terrainMesh;
	};
}