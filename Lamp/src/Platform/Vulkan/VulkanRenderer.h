#pragma once

#include "Lamp/Rendering/RendererNew.h"
#include "Lamp/Rendering/RendererDataStructures.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class RenderPipeline;
	class Shader;
	class CommandBuffer;
	class UniformBufferSet;
	class Mesh;
	class Texture2D;

	struct TempRendererStorage
	{
		Ref<CommandBuffer> swapchainCommandBuffer;
		Ref<CommandBuffer> renderCommandBuffer;

		MeshDataBuffer meshBuffer;

		Ref<RenderPipeline> currentRenderPipeline;
	};

	class VulkanRenderer : public RendererNew
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void Initialize() override;
		void Shutdown() override;
		void Begin(const Ref<CameraBase> camera) override;
		void End() override;

		void BeginPass(Ref<RenderPipeline> pipeline) override;
		void EndPass() override;

		void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id /* = -1 */);

		inline VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }

	private:

		Scope<TempRendererStorage> m_rendererStorage;
		VkDescriptorPool m_descriptorPool;
	};
}