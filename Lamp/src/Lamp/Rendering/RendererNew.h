#pragma once

namespace Lamp
{
	class CameraBase;
	class SubMesh;
	class Material;
	class RenderPipeline;

	class RendererNew
	{
	public:
		virtual void Initialize() = 0;;
		virtual void Shutdown() = 0;
		virtual void Begin(const Ref<CameraBase> camera) = 0;
		virtual void End() = 0;

		virtual void BeginPass(Ref<RenderPipeline> pipeline) = 0;
		virtual void EndPass() = 0;

		virtual const uint64_t GetMemoryUsage() const = 0;

		virtual void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id = -1) = 0;

		static Ref<RendererNew> Create();
	private:
	};
}