#pragma once

#include "Lamp/Rendering/Buffers/VertexBuffer.h"

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
		virtual void SetLayout(BufferLayout layout) = 0;
		
		static Ref<RenderPipeline> Create(const RenderPipelineSpecification& specification);

	private:
	};
}