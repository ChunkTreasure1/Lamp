#pragma once
#include "Cameras/PerspectiveCamera.h"
#include "Lamp/Mesh/SubMesh.h"
#include "Lamp/Mesh/Materials/Material.h"

#include "Lamp/Rendering/Vertices/Framebuffer.h"
#include "Lamp/Core/GlobalEnvironment.h"
#include "Buffers/RenderBuffer.h"

namespace Lamp
{	
	class RenderGraph;

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

		static void Begin(const Ref<CameraBase> camera);
		static void End();

		static void BeginPass(RenderPassSpecification& passSpec);
		static void EndPass();

		static void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> mat, size_t id = -1);

		static void DrawRenderBuffer();

		static void DrawSkybox();
		static void DrawCube();
		static void DrawQuad();

		static void RenderQuad();

		static void SetEnvironment(const std::string& path);

	private:
		static void CreateBaseMeshes();

		static void DrawMesh(const glm::mat4& modelMatrix, const Ref<VertexArray> data, const Ref<Material> mat, size_t id = -1);

	private:
		static RenderBuffer s_RenderBuffer;
	};
}