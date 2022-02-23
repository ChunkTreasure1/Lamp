#pragma once

#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/RenderPipelineCompute.h"

namespace Lamp
{
	struct RenderPass
	{
		Ref<RenderPipeline> graphicsPipeline;
		Ref<RenderComputePipeline> computePipeline;
		
		std::function<void()> computeExcuteCommand;
	};
}