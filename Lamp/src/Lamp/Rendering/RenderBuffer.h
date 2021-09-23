#pragma once

#include "Lamp/Meshes/SubMesh.h"
#include "Lamp/Meshes/Materials/Material.h"

namespace Lamp
{
	enum class RenderData
	{
		Transform,
		Data,
		Material,
		ID
	};

	struct RenderCommandData
	{
		RenderCommandData() {}

		glm::mat4 transform;
		Ref<VertexArray> data;
		Material material;
		size_t id;
	};

	struct RenderBuffer
	{
		std::vector<RenderCommandData> drawCallsDeferred;
		std::vector<RenderCommandData> drawCallsForward;
	};
}