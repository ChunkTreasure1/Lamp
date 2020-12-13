#include "BoundingSphere.h"
#include "lppch.h"

#include "Lamp/Math/Math.h"

namespace Lamp {
IntersectData BoundingSphere::IntersectRay(const Ray &ray) const {
  float t0 = 0.f;
  float t1 = 0.f;

  glm::vec3 pos = m_Center;

  glm::vec3 l = ray.origin - pos;
  float a = glm::dot(ray.direction, ray.direction);
  float b = 2.f * glm::dot(ray.direction, l);
  float c = glm::dot(l, l) - (m_Radius * m_Radius);

  if (!Math::SolveQuadric(a, b, c, t0, t1)) {
    return IntersectData(false, glm::vec3(0.f));
  }

  if (t0 > t1) {
    std::swap(t0, t1);
  }

  if (t0 < 0) {
    t0 = t1;
    if (t0 < 0) {
      return IntersectData(false, glm::vec3(0.f));
    }
  }

  LP_CORE_INFO("Raycast Hit!");
  return IntersectData(true, glm::vec3(0.f));
}

IntersectData BoundingSphere::Intersect(const Ref<Collider> &other) const {
  if (other->GetType() == CollType::Sphere) {
    Ref<BoundingSphere> coll = std::dynamic_pointer_cast<BoundingSphere>(other);

    float radDist = m_Radius + coll->GetRadius();
    float centerDist = glm::length(coll->GetCenter() - m_Center);

    glm::vec3 direction = coll->GetCenter() - m_Center;
    direction /= centerDist;

    float distance = centerDist - radDist;

    return IntersectData(distance < 0, direction * distance);
  } else if (other->GetType() == CollType::AABB) {

  } else if (other->GetType() == CollType::Plane) {
  }

  return IntersectData(false, glm::vec3(0.f));
}

void BoundingSphere::Transform(const glm::vec3 &translation) {
  m_Center += translation;
}
} // namespace Lamp
