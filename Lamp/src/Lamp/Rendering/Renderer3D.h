#pragma once
#include "Cameras/PerspectiveCamera.h"
#include "Lamp/Meshes/Mesh.h"
#include "Lamp/Meshes/Material.h"

namespace Lamp
{
	class Renderer3D
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(PerspectiveCamera& camera);
		static void End();

		static void TestDraw();
		static void DrawMesh(const glm::mat4& modelMatrix, Mesh& mesh, Material& mat);

	};
}