#pragma once

#include "Lamp/Rendering/Buffers/VertexBuffer.h"
#include "Lamp/Rendering/Buffers/UniformBufferSet.h"

namespace Lamp
{
	class Shader;
	class CommandBuffer;

	enum class Topology
	{
		TriangleList,
		LineList,
		TriangleStrip
	};

	struct RenderPipelineSpecification
	{
		Ref<Framebuffer> framebuffer;
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
		virtual void Bind(Ref<CommandBuffer> commandBuffer) const = 0;
		virtual void SetLayout(BufferLayout layout) = 0;
		virtual const RenderPipelineSpecification& GetSpecification() const = 0;
		
		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification);
	};
}