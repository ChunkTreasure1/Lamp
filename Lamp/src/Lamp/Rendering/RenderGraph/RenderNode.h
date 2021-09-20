#pragma once

#include <string>

#include "Lamp/Rendering/RenderPass.h"

namespace Lamp
{
	struct RenderLink
	{
		Ref<RenderPass> inputPass;
		Ref<RenderPass> outputPass;
	
		uint32_t id;
	};

	struct RenderNode
	{
		std::string name;
		glm::vec2 position;

		Ref<RenderLink> link;
		Ref<RenderPass> renderPass;
	
		uint32_t id;

		void Render(Ref<CameraBase>& camera)
		{
			renderPass->Render(camera);
		}
	};
}