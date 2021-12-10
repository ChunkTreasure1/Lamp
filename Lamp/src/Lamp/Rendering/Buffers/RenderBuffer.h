#pragma once

#include "Lamp/Mesh/SubMesh.h"
#include "Lamp/Mesh/Materials/Material.h"

namespace Lamp
{
	enum class RenderData
	{
		Transform,
		Data,
		MaterialBlendingMultiplier,
		ID,
		MaterialUseBlending
	};

	struct RenderCommandData
	{
		RenderCommandData() 
			: id(0), transform(glm::mat4(1.f))
		{}

		RenderCommandData(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
			: transform(transform), data(mesh), material(material), id(id)
		{}

		glm::mat4 transform;
		Ref<SubMesh> data;
		Ref<Material> material;
		size_t id;
	};

	struct RenderBuffer
	{
		std::vector<RenderCommandData> drawCalls;
	};
}