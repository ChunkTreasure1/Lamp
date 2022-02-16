#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct Vertex
	{
		Vertex(const glm::vec3& position, const glm::vec2& texCoords)
			: position(position), textureCoords(texCoords)
		{
		}

		Vertex(const glm::vec3& position)
			: position(position)
		{
		}

		Vertex() = default;

		glm::vec3 position = glm::vec3(0.f);
		glm::vec3 normal = glm::vec3(0.f);
		glm::vec3 tangent = glm::vec3(0.f);
		glm::vec3 bitangent = glm::vec3(0.f);
		glm::vec2 textureCoords = glm::vec2(0.f);
	};

	struct QuadVertex
	{
		glm::vec3 position = glm::vec3(0.f);
		glm::vec4 color = glm::vec4(0.f);
		glm::vec2 textureCoords = glm::vec2(0.f);
		uint32_t textureId;
		uint32_t id;
	};

	struct LineVertex
	{
		glm::vec3 position = glm::vec3(0.f);
	};
}