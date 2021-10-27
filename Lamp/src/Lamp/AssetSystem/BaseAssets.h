#pragma once

#include "Asset.h"

namespace Lamp
{
	class MeshSource : public Asset
	{
	public:
		MeshSource(const std::filesystem::path& path, AssetHandle mesh)
			: m_sourceMesh(path), m_mesh(mesh)
		{}

		MeshSource()
		{}

		static AssetType GetStaticType() { return AssetType::MeshSource; }
		virtual AssetType GetType() override { return GetStaticType(); }

		friend class MeshSourceLoader;

	private:
		std::filesystem::path m_sourceMesh;
		AssetHandle m_mesh;
	};
}