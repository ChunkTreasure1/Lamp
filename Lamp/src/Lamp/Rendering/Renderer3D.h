#pragma once
#include "Cameras/PerspectiveCamera.h"
#include "Lamp/Meshes/SubMesh.h"
#include "Lamp/Meshes/Materials/Material.h"

#include "Lamp/Rendering/Vertices/Framebuffer.h"
#include "Lamp/Core/GlobalEnvironment.h"
#include "RenderBuffer.h"

namespace Lamp
{	
	struct RendererSettings
	{
		int SSAOKernelSize = 64;
		const int SSAOMaxKernelSize = 64; //Needs to be matched with value in ssao shader

		//SSAO
		float SSAORadius = 0.5f;
		float SSAOBias = 0.025f;
		Ref<Texture2D> SSAONoiseTexture;
		
		float HDRExposure = 3.f;
		float Gamma = 2.2f;
	};

	enum class ERendererSettings
	{
		SSAOKernelSize,
		SSAOMaxKernelSize,
		SSAORadius,
		SSAOBias,
		HDRExposure,
		Gamma
	};

	struct RenderPassSpecification;
	class Renderer3D
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void Begin(Ref<CameraBase>& camera);
		static void End();

		static void BeginPass(RenderPassSpecification& passSpec);
		static void EndPass();

		static void Flush();

		static void RegenerateSSAOKernel();

		static void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh>& mesh, const Material& mat, size_t id = -1);
		static void SubmitMeshForward(const glm::mat4& transform, const Ref<SubMesh>& mesh, const Material& mat, size_t id = -1);

		static void SubmitQuadForward(const glm::mat4& transform, const Material& mat, size_t id = -1);

		static void DrawRenderBuffer();

		static void DrawLine(const glm::vec3& posA, const glm::vec3& posB, float width);
		static void DrawSkybox();
		static void DrawCube();
		static void DrawQuad();

		static void RenderQuad();

		static void SetEnvironment(const std::string& path);
		static RendererSettings& GetSettings() { return s_RendererSettings; }

	private:
		static void StartNewBatch();
		static void ResetBatchData();

		static void DrawMesh(const glm::mat4& modelMatrix, Ref<VertexArray>& data, Material& mat, size_t id = -1);
		static void DrawMeshForward(const glm::mat4& modelMatrix, Ref<VertexArray>& data, Material& mat, size_t id = -1);

	private:
		static RendererSettings s_RendererSettings;
		static RenderBuffer s_RenderBuffer;
	};
}