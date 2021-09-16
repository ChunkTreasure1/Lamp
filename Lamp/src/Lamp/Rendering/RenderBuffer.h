#pragma once

#include "Lamp/Meshes/SubMesh.h"
#include "Lamp/Meshes/Materials/Material.h"

namespace Lamp
{
	struct RenderSubmitData
	{
		RenderSubmitData() {}

		glm::mat4 transform;
		Ref<VertexArray> data;
		Material material;
		size_t id;
	};

	struct RenderBuffer
	{
		std::vector<RenderSubmitData> drawCallsDeferred;
		std::vector<RenderSubmitData> drawCallsForward;
	};
}