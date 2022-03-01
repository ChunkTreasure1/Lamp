#pragma once

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/RenderPipeline.h"

#include <unordered_map>

namespace Lamp
{
	class RenderPipelineLibrary
	{
	public:
		RenderPipelineLibrary();
		~RenderPipelineLibrary();

		Ref<RenderPipeline> GetPipeline(ERenderPipeline pipeline);
		ERenderPipeline GetTypeFromPipeline(Ref<RenderPipeline> pipeline);

		static RenderPipelineLibrary& Get();

	private:
		void SetupRenderPipelines();

		static RenderPipelineLibrary* s_instance;
		
		std::unordered_map<ERenderPipeline, Ref<RenderPipeline>> m_renderPipelines;
	};
}