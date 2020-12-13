#pragma once

#include <glm/glm.hpp>

namespace Lamp::Math
{
bool DecomposeTransform(const glm::mat4& transform, glm::vec3& trans, glm::vec3& rot, glm::vec3& scale);

bool SolveQuadric(const float& a, const float& b, const float& c, float& x0, float& x1);

float Max(const glm::vec3& v);
}