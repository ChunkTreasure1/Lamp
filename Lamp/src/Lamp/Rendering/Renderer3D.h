#pragma once
#include "Cameras/PerspectiveCamera.h"
#include "Lamp/Meshes/Mesh.h"
#include "Lamp/Meshes/Material.h"

#include "Lamp/Rendering/Vertices/FrameBuffer.h"

namespace Lamp
{
	class Renderer3D
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(PerspectiveCamera& camera, bool isMain = true);
		static void End();
		static void Flush();

		static void DrawMesh(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat);
		static void DrawSkybox();
		static void DrawLine(const glm::vec3& posA, const glm::vec3& posB, float width);

		static Ref<FrameBuffer>& GetFrameBuffer() { return m_pFrameBuffer; }

	private:
		static Ref<FrameBuffer> m_pFrameBuffer;

		static void StartNewBatch();
		static void ResetBatchData();
	};
}