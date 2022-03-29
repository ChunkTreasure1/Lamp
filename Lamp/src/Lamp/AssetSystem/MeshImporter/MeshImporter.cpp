#include "lppch.h"
#include "MeshImporter.h"

#include "FbxImporter.h"
#include "AssimpImporter.h"

namespace Lamp
{
	std::unordered_map<MeshFormat, Scope<MeshImporter>> MeshImporter::s_importers;

	namespace Utility
	{
		MeshFormat FormatFromExtension(const std::filesystem::path& aPath)
		{
			if (aPath.extension().string() == ".fbx" || aPath.extension().string() == ".FBX")
			{
				return MeshFormat::Fbx;
			}
			else if (aPath.extension().string() == ".gltf" || aPath.extension().string() == ".glb")
			{
				return MeshFormat::GLTF;
			}
			else
			{
				return MeshFormat::Other;
			}
		}
	}

	void MeshImporter::Initialize()
	{
		s_importers[MeshFormat::Fbx] = CreateScope<FbxImporter>();
		s_importers[MeshFormat::Other] = CreateScope<AssimpImporter>();
	}

	void MeshImporter::Shutdown()
	{
		s_importers.clear();
	}

	Ref<Mesh> MeshImporter::ImportMesh(const MeshImportSettings& settings)
	{
		return s_importers[Utility::FormatFromExtension(settings.path)]->ImportMesh(settings);
	}
}