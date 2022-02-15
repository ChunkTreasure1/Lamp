#pragma once

#include "Asset.h"

#include "Lamp/AssetSystem/MeshImporter.h"

namespace Lamp
{
	class MeshSource : public Asset
	{
	public:
		MeshSource(const MeshImportSettings& settings, AssetHandle mesh)
			: m_importSettings(settings), m_mesh(mesh)
		{}

		MeshSource()
		{}

		const std::filesystem::path& GetSource() const { return m_importSettings.path; }
		const AssetHandle& GetMeshAsset() const { return m_mesh; }
		const MeshImportSettings& GetImportSettings() const { return m_importSettings; }

		static AssetType GetStaticType() { return AssetType::MeshSource; }
		AssetType GetType() override { return GetStaticType(); }

		friend class MeshSourceLoader;

	private:
		AssetHandle m_mesh;
		MeshImportSettings m_importSettings;
	};
}