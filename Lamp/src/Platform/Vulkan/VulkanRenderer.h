#pragma once

#include "Lamp/Rendering/RendererNew.h"
#include "Lamp/Rendering/RendererDataStructures.h"

#include <vulkan/vulkan_core.h>
#include <atomic>

namespace Lamp
{
	class RenderPipeline;
	class Shader;
	class CommandBuffer;
	class UniformBufferSet;
	class Mesh;
	class Texture2D;

	struct VulkanRendererStorage
	{
		Ref<CommandBuffer> swapchainCommandBuffer;
		Ref<CommandBuffer> renderCommandBuffer;

		MeshDataBuffer meshBuffer;

		Ref<RenderPipeline> currentRenderPipeline;
		Ref<CameraBase> camera;

		Ref<SubMesh> quadMesh;
		Ref<SubMesh> cubeMesh;

		std::vector<VkDescriptorSet> pipelineDescriptorSets;
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

		const GPUMemoryStatistics& GetMemoryUsage() const override;

		void SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id /* = -1 */);
		void SubmitQuad() override;
		void SubmitCube(void* pushConstant) override;

		void DrawBuffer(RenderBuffer& buffer) override;

		inline VkDescriptorPool GetDescriptorPool() { return m_descriptorPool; }

	private:
		void SetupDescriptorsForMaterialRendering(Ref<Material> material);
		void SetupDescriptorsForQuadRendering(); // TODO: should be moved into singular function
		void SetupDescriptorsForCubeRendering(); // TODO: should be moved into singular function

		void SortRenderBuffer(const glm::vec3& sortPoint, RenderBuffer& buffer);
		void CreateBaseGeometry();

		Scope<VulkanRendererStorage> m_rendererStorage;
		VkDescriptorPool m_descriptorPool;

		std::atomic_bool m_renderingFinished = true;
	};
}