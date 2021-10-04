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
		RenderCommandData() 
			: id(0), transform(glm::mat4(1.f))
		{}

		glm::mat4 transform;
		Ref<VertexArray> data;
		Ref<Material> material;
		size_t id;
	};

	struct RenderBuffer
	{
		std::vector<RenderCommandData> drawCalls;
	};
}