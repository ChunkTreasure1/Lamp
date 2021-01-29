#pragma once
#include "Cameras/PerspectiveCamera.h"
#include "Lamp/Meshes/Mesh.h"
#include "Lamp/Meshes/Materials/Material.h"

#include "Lamp/Rendering/Vertices/Framebuffer.h"
#include "Lamp/Core/GlobalEnvironment.h"


namespace Lamp
{	
	struct RenderPassSpecification;
	class Renderer3D
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(const RenderPassSpecification& passSpec);
		static void End();
		static void Flush();

		static void DrawMesh(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat, size_t id = -1);
		static void DrawLine(const glm::vec3& posA, const glm::vec3& posB, float width);

		static void DrawSkybox();
		static void DrawCube();
		static void DrawQuad();
		static void DrawGrid();

	private:
		static void StartNewBatch();
		static void ResetBatchData();
	};
}