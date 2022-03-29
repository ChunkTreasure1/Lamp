#pragma once

#include "Lamp/Core/Core.h"

#include <glm/glm.hpp>

#include <filesystem>

namespace Lamp
{
	enum class MeshUnit
	{
		Centimeters = 0,
		Decimeters = 1,
		Meters = 2
	};
	
	enum class MeshAxis
	{
		XP = 0,
		XN = 1,
		YP = 2,
		YN = 3,
		ZP = 4,
		ZN = 5
	};
	
	enum class MeshFormat
	{
		Fbx = 0,
		GLTF = 1,
		Other = 2
	};

	struct MeshImportSettings
	{
		std::filesystem::path path;
		bool compileStatic = false;
		MeshUnit units = MeshUnit::Centimeters;
		
		MeshAxis upAxis = MeshAxis::YP;
		MeshAxis forwardAxis = MeshAxis::ZN;
	};

	class Mesh;
	class MeshImporter
	{
	public:
		virtual ~MeshImporter() = default;

		static void Initialize();
		static void Shutdown();
		static Ref<Mesh> ImportMesh(const MeshImportSettings& settings);

	protected:
		virtual Ref<Mesh> ImportMeshImpl(const MeshImportSettings& settings) = 0;
		
	private:
		static std::unordered_map<MeshFormat, Scope<MeshImporter>> s_importers;
	};
}