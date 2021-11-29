#pragma once

#include "Lamp/Rendering/Buffers/BufferLayout.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	class Texture2D;
	class Shader;

	enum class Topology
	{
		TriangleList,
		LineList,
		TriangleStrip
	};

	struct RenderPipelineSpecification
	{
		Ref<Shader>	shader;
		//Ref<UniformBufferSet> uniformBufferSets;

		Topology topology;
		BufferLayout vertexLayout;
		glm::vec2 size;

		bool isSwapchain;
	};

	class RenderPipeline
	{
	public:
		virtual ~RenderPipeline() = 0;

		virtual void Bind(uint32_t index) const = 0;
		virtual void BindDescriptors(uint32_t index) const = 0;

		virtual void SetLayout(BufferLayout layout) = 0;
		virtual void SetTexture(Ref<Texture2D> texture, uint32_t binding, uint32_t set, uint32_t index) = 0;

		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification);
	};
}