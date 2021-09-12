#pragma once
#include "Cameras/PerspectiveCamera.h"
#include "Lamp/Meshes/Mesh.h"
#include "Lamp/Meshes/Materials/Material.h"

#include "Lamp/Rendering/Vertices/Framebuffer.h"
#include "Lamp/Core/GlobalEnvironment.h"


namespace Lamp
{	
	struct RendererSettings
	{
		int SSAOKernelSize = 64;
		const int SSAOMaxKernelSize = 256; //Needs to be matched with value in ssao shader

		float SSAORadius = 0.5f;
		float SSAOBias = 0.025f;
		float HDRExposure = 3.f;
		float Gamma = 2.2f;
	};

	struct RenderPassSpecification;
	class Renderer3D
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(RenderPassSpecification& passSpec, Ref<CameraBase>& camera);
		static void End();
		static void CombineLightning();
		static void Flush();
		static void CopyDepth();

		static void SSAOMainPass();
		static void SSAOBlurPass();
		static void RegenerateSSAOKernel();

		static void DrawMesh(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat, size_t id = -1);
		static void DrawMeshForward(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat, size_t id = -1);
		static void DrawLine(const glm::vec3& posA, const glm::vec3& posB, float width);

		static void DrawSkybox();
		static void DrawCube();
		static void DrawQuad();
		static void DrawGrid();

		static void SetEnvironment(const std::string& path);
		static RendererSettings& GetSettings() { return s_RendererSettings; }

	private:
		static void StartNewBatch();
		static void ResetBatchData();

	private:
		static RendererSettings s_RendererSettings;
	};
}