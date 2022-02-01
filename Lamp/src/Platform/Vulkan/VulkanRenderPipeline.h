#pragma once

#include "Lamp/Rendering/RenderPipeline.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class Texture2D;
	class TextureHDR;
	class Image2D;

	class VulkanRenderPipeline : public RenderPipeline
	{
	public:
		VulkanRenderPipeline(const RenderPipelineSpecification& specification);
		~VulkanRenderPipeline();

		void Bind(Ref<CommandBuffer> commandBuffer) const override;
		void SetLayout(BufferLayout layout) override;

		void BindDescriptorSets(Ref<CommandBuffer> commandBuffer, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t startSet = 0) const;
		void BindDescriptorSet(Ref<CommandBuffer> commandBuffer, VkDescriptorSet descriptorSet, uint32_t set) const;

		void SetTexture(Ref<Texture2D> texture, uint32_t binding, uint32_t set, uint32_t index);
		void SetTexture(Ref<TextureHDR> texture, uint32_t binding, uint32_t set, uint32_t index);
		void SetTexture(Ref<Image2D> image, uint32_t set, uint32_t binding, uint32_t index);

		void SetPushConstantData(Ref<CommandBuffer> commandBuffer, uint32_t index, const void* data);
		const RenderPipelineSpecification& GetSpecification() const override { return m_specification; }

		inline const uint32_t GetDescriptorSetCount() const { return (uint32_t)m_descriptorSets.at(0).size(); }

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