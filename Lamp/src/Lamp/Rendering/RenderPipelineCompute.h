#pragma once

#include <vulkan/vulkan.h>

namespace Lamp
{
	class Shader;
	class CommandBuffer;


	class RenderComputePipeline
	{
	public:
		RenderComputePipeline(Ref<Shader> computeShader);
		~RenderComputePipeline() = default;

		void Begin(Ref<CommandBuffer> commandBuffer = nullptr);
		void End();
		Ref<Shader> GetShader() { return m_shader; }

		void Execute(VkDescriptorSet * descriptorSets, uint32_t descriptorSetCount, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		void Dispatch(VkDescriptorSet descriptorSet, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

		VkCommandBuffer GetActiveCommandBuffer() { return m_activeComputeCommandBuffer; }

		void SetPushConstants(const void* data, uint32_t size);
		void CreatePipeline();

		static Ref<RenderComputePipeline> Create(Ref<Shader> computeShader);

	private:
		Ref<Shader> m_shader;

		VkPipelineLayout m_computePipelineLayout = nullptr;
		VkPipelineCache m_computePipelineCache = nullptr;
		VkPipeline m_computePipeline = nullptr;

		VkCommandBuffer m_activeComputeCommandBuffer = nullptr;

		bool m_usingGraphicsQueue = false;
	};
}