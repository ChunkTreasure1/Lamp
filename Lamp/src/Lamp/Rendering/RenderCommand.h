#pragma once

#include "Lamp/Core/Core.h"

#include "Lamp/Rendering/Buffers/RenderBuffer.h"

#include <vulkan/vulkan.h>

namespace Lamp
{
	class Renderer;
	class Renderer2D;
	class CameraBase;
	class RenderPipeline;
	class Material;
	class SubMesh;
	class Texture2D;
	class MaterialInstance;

	class RenderCommand
	{
	public:
		static void Initialize(Renderer* renderer, Renderer2D* renderer2D);
		static void Shutdown();

		static void Begin(const Ref<CameraBase> camera);
		static void End();
		static void SwapRenderBuffers();

		static void BeginPass(const Ref<RenderPipeline> pipeline);
		static void EndPass();

		static void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<MaterialInstance> material, size_t id = -1);
		static void SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant = nullptr);
		static void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, size_t id = -1);
		static void SubmitLine(const glm::vec3& pointOne, const glm::vec3& pointTwo, const glm::vec4& color);

		static void DispatchRenderCommands();

	private:
		RenderCommand() = delete;

		static Renderer* s_renderer;
		static Renderer2D* s_renderer2D;
	};
}