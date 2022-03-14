#pragma once

#include <glm/glm.hpp>

namespace Lamp::Math
{
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& trans, glm::vec3& rot, glm::vec3& scale);
	bool SolveQuadric(const float& a, const float& b, const float& c, float& x0, float& x1);
	float Max(const glm::vec3& v);
	
	inline float Lerp(float a, float b, float t)
	{
		return (1.f - t) * a + t * b;
	}

	inline glm::vec2 Lerp(const glm::vec2& a, const glm::vec2& b, float t)
	{
		return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) };
	}

	inline glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t)
	{
		return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
	}
}