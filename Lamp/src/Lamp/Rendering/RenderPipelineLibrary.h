#pragma once

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/RenderPipeline.h"

#include <unordered_map>

namespace Lamp
{
	class RenderPipelineLibrary
	{
	public:
		static void Initialize();
		static void Shutdown();

		static Ref<RenderPipeline> GetPipeline(const std::string& pipeline);
		
		static std::string GetTypeFromPipeline(Ref<RenderPipeline> pipeline);
		static std::vector<std::string> GetPipelineNames();
		
	private:
		static void SetupRenderPipelines();
		
		static std::unordered_map<std::string, Ref<RenderPipeline>> s_renderPipelines;
	};
}