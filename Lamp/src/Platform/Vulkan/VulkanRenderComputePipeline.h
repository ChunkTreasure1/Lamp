#pragma once

#include "Lamp/Rendering/RenderPipeline.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class VulkanShader;
	class VulkanRenderComputePipeline : public RenderComputePipeline
	{
	public:
		VulkanRenderComputePipeline(Ref<Shader> computeShader);
		~VulkanRenderComputePipeline() override = default;

		void Begin(Ref<CommandBuffer> commandBuffer = nullptr) override;
		void End();
		Ref<Shader> GetShader() override { return m_shader; }

		void Execute(VkDescriptorSet* descriptorSets, uint32_t descriptorSetCount, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		void Dispatch(VkDescriptorSet descriptorSet, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		VkCommandBuffer GetActiveCommandBuffer() { return m_activeComputeCommandBuffer; }

		void SetPushConstants(const void* data, uint32_t size);
		void CreatePipeline();

	private:
		Ref<Shader> m_shader;

		VkPipelineLayout m_computePipelineLayout = nullptr;
		VkPipelineCache m_computePipelineCache = nullptr;
		VkPipeline m_computePipeline = nullptr;

		VkCommandBuffer m_activeComputeCommandBuffer = nullptr;
	
		bool m_usingGraphicsQueue = false;
	};
}