#pragma once

#include "Lamp/Rendering/Buffers/VertexBuffer.h"
#include "Lamp/Rendering/Buffers/UniformBufferSet.h"

namespace Lamp
{
	class Shader;

	enum class Topology
	{
		TriangleList,
		LineList,
		TriangleStrip
	};

	struct RenderPipelineSpecification
	{
		Ref<Shader> shader;
		Ref<UniformBufferSet> uniformBufferSets;

		Topology topology;
		BufferLayout vertexLayout;

		glm::vec2 size;
		bool isSwapchain;
	};

	class RenderPipeline
	{
	public:
		virtual void Bind(uint32_t index) const = 0;
		virtual void SetLayout(BufferLayout layout) = 0;
		
		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification);
	};
}