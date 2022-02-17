#pragma once

#include "Lamp/Mesh/SubMesh.h"
#include "Lamp/Mesh/Materials/Material.h"

namespace Lamp
{
	struct RenderCommandData
	{
		RenderCommandData() 
			: id(0), transform(glm::mat4(1.f))
		{}

		RenderCommandData(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
			: transform(transform), data(mesh), material(material), id(id)
		{}

		Ref<SubMesh> data;
		Ref<Material> material;

		glm::vec4 color; //2D only
		Ref<Texture2D> texture; //2D only

		glm::mat4 transform;
		size_t id;
	};

	struct LineCommandData
	{
		glm::vec3 pointOne;
		glm::vec3 pointTwo;
	};

	struct RenderBuffer
	{
		std::vector<RenderCommandData> drawCalls;
		std::vector<LineCommandData> lineCalls;
	};
}