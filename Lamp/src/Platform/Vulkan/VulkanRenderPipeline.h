#pragma once

#include "Lamp/Rendering/RenderPipeline.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class Texture2D;
	class VulkanRenderPipeline : public RenderPipeline
	{
	public:
		VulkanRenderPipeline(const RenderPipelineSpecification& specification);
		~VulkanRenderPipeline() override;

		void Bind(uint32_t index) const override;
		void SetLayout(BufferLayout layout) override;

		void BindDescriptorSets(uint32_t index) const;

		void SetLayout(BufferLayout layout);
		void SetTexture(Ref<Texture2D> texture, uint32_t binding, uint32_t set, uint32_t index);

	private:
		void CreateDescriptorSets();
		void SetupUniformBuffers();

		void Invalidate();

		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
		VkVertexInputBindingDescription m_bindingDescription;

		uint32_t m_numAttributes;

		VkPipelineLayout m_layout = nullptr;
		VkPipeline m_pipeline = nullptr;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> m_descriptorSets; //frame->descriptor sets

		RenderPipelineSpecification m_specification;
	};
}