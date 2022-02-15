#pragma once

#include "Lamp/Core/Core.h"

#include "Lamp/Rendering/Buffers/RenderBuffer.h"

#include <vulkan/vulkan.h>

namespace Lamp
{
	class Renderer;
	class CameraBase;
	class RenderPipeline;
	class Material;
	class SubMesh;

	class RenderCommand
	{
	public:
		static void Initialize(Renderer* renderer);
		static void Shutdown();

		static void Begin(const Ref<CameraBase> camera);
		static void End();
		static void SwapRenderBuffers();

		static void BeginPass(const Ref<RenderPipeline> pipeline);
		static void EndPass();

		static void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id = -1);
		static void SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant = nullptr);

		static void DispatchRenderCommands();

	private:
		RenderCommand() = delete;

		static Renderer* s_renderer;
	};
}