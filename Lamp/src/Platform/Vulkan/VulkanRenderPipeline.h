#pragma once

#include "Lamp/Rendering/RenderPipeline.h"

namespace Lamp
{
	class VulkanRenderPipeline : public RenderPipeline
	{
	public:
		VulkanRenderPipeline(const RenderPipelineSpecification& specification);
		~VulkanRenderPipeline() override;

		void Bind(uint32_t index) const override;
		void BindDescriptors(uint32_t index) const override;

		void SetLayout(BufferLayout layout) override;
		void SetTexture(Ref<Texture2D> texture, uint32_t binding, uint32_t set, uint32_t index) override;

	private:

	};
}