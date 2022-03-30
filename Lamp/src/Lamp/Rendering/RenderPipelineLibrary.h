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
		Ref<RenderPipeline> GetPipeline(const std::string& pipeline);
		
		ERenderPipeline GetTypeFromPipeline(Ref<RenderPipeline> pipeline);
		std::vector<std::string> GetPipelineNames() const;
		

		static RenderPipelineLibrary& Get();

	private:
		void SetupRenderPipelines();

		static RenderPipelineLibrary* s_instance;
		
		std::unordered_map<ERenderPipeline, Ref<RenderPipeline>> m_renderPipelines;
		std::unordered_map<std::string, ERenderPipeline> m_renderPipelinesNameMap;
	};
}