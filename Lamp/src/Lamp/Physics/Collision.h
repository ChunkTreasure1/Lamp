#pragma once

#include "Components/ConvexPolygonCollider.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Event/EntityEvent.h"
#include "Lamp/Level/LevelSystem.h"

namespace Lamp {
class Collision {
public:
  static std::pair<bool, float>
  SAT(Ref<ConvexPolygonColliderComponent> &pCollA,
      Ref<ConvexPolygonColliderComponent> &pCollB) {
    auto polyA = pCollA;
    auto polyB = pCollB;

    float overlap = INFINITY;

    for (size_t i = 0; i < 2; i++) {
      if (i == 1) {
        polyA = pCollB;
        polyB = pCollA;
      }

      for (size_t a = 0; a < polyA->GetVertices().size(); a++) {
        size_t b = (a + 1) % polyA->GetVertices().size();
        glm::vec2 axisProj = polyA->GetVertices()[a] - polyA->GetVertices()[b];

        float d = sqrtf(axisProj.x * axisProj.x + axisProj.y * axisProj.y);
        axisProj = {axisProj.x / d, axisProj.y / d};

        float min_r1 = INFINITY, max_r1 = -INFINITY;
        for (size_t p = 0; p < polyA->GetVertices().size(); p++) {
          float q = glm::dot(polyA->GetVertices()[p], axisProj);
          min_r1 = std::min(min_r1, q);
          max_r1 = std::max(max_r1, q);
        }

        float min_r2 = INFINITY, max_r2 = -INFINITY;
        for (size_t p = 0; p < polyB->GetVertices().size(); p++) {
          float q = glm::dot(polyB->GetVertices()[p], axisProj);
          min_r2 = std::min(min_r2, q);
          max_r2 = std::max(max_r2, q);
        }

        overlap = std::min(std::min(max_r1, max_r2) - std::max(min_r1, min_r2),
                           overlap);
        if (!(max_r2 >= min_r1 && max_r1 >= min_r2)) {
          return std::make_pair<>(false, 0.f);
        }
      }
    }

    // glm::vec2 vec = pCollB->GetOwner()->GetPosition() -
    // pCollA->GetOwner()->GetPosition(); float s = sqrtf(vec.x * vec.x + vec.y *
    // vec.y);

    // if (pCollA->GetMass() > pCollB->GetMass())
    //{
    //	pCollB->GetOwner()->SetPosition(pCollB->GetOwner()->GetPosition() -
    //glm::vec3((overlap * vec.x / s), (overlap * vec.y / s), 0));
    //}
    // else
    //{
    //	pCollA->GetOwner()->SetPosition(pCollA->GetOwner()->GetPosition() -
    //glm::vec3((overlap * vec.x / s), (overlap * vec.y / s), 0));
    //}

    return std::make_pair<>(true, overlap);
  }

  static void CheckCollisions() {
    /*for (size_t m = 0; m <
    LevelSystem::GetCurrentLevel()->GetEntityManager()->GetEntities().size();
    m++)
    {
            for (size_t n = m + 1; n <
    LevelSystem::GetCurrentLevel()->GetEntityManager()->GetEntities().size();
    n++)
            {
                    if (auto pCollA =
    LevelSystem::GetCurrentLevel()->GetEntityManager()->GetEntities()[m]->GetComponent<ConvexPolygonColliderComponent>())
                    {
                            if (auto pCollB =
    LevelSystem::GetCurrentLevel()->GetEntityManager()->GetEntities()[n]->GetComponent<ConvexPolygonColliderComponent>())
                            {
                                    std::pair<bool, float> pair;
                                    pair = SAT(pCollA, pCollB);
                                    pCollA->SetIsColliding(pair.first);
                                    pCollB->SetIsColliding(pair.first);
                                    if (pCollA->GetIsColliding())
                                    {
                                            if (auto it =
    std::find(pCollA->GetCurrentColliders().begin(),
    pCollA->GetCurrentColliders().end(), pCollB); it ==
    pCollA->GetCurrentColliders().end())
                                            {
                                                    pCollA->GetCurrentColliders().push_back(pCollB);
                                                    {
                                                            if
    (pCollB->GetOwner() != NULL)
                                                            {
                                                                    EntityCollisionEvent
    event(pair.second, pCollB->GetOwner(), pCollB->GetTag());
                                                                    pCollA->GetOwner()->OnEvent(event);
                                                            }
                                                            else
                                                            {
                                                                    EntityCollisionEvent
    event(pair.second, nullptr, "null"); pCollA->GetOwner()->OnEvent(event);
                                                            }
                                                    }
                                            }

                                            if (auto it =
    std::find(pCollB->GetCurrentColliders().begin(),
    pCollB->GetCurrentColliders().end(), pCollA); it ==
    pCollB->GetCurrentColliders().end())
                                            {
                                                    if (pCollA->GetOwner() !=
    NULL)
                                                    {
                                                            EntityCollisionEvent
    event(pair.second, pCollA->GetOwner(), pCollA->GetTag());
                                                            pCollB->GetOwner()->OnEvent(event);
                                                    }
                                                    else
                                                    {
                                                            EntityCollisionEvent
    event(pair.second, nullptr, "null"); pCollB->GetOwner()->OnEvent(event);
                                                    }
                                            }
                                    }
                                    else if (!pCollA->GetIsColliding() &&
    !pCollB->GetIsColliding())
                                    {
                                            if (auto it =
    std::find(pCollA->GetCurrentColliders().begin(),
    pCollA->GetCurrentColliders().end(), pCollB); it !=
    pCollA->GetCurrentColliders().end())
                                            {
                                                    pCollA->GetCurrentColliders().erase(it);

                                                    if (pCollB->GetOwner() !=
    NULL)
                                                    {
                                                            EntityStoppedCollisionEvent
    event(pair.second, pCollB->GetOwner(), pCollB->GetTag());
                                                            pCollA->GetOwner()->OnEvent(event);
                                                    }
                                                    else
                                                    {
                                                            EntityStoppedCollisionEvent
    event(pair.second, nullptr, ""); pCollA->GetOwner()->OnEvent(event);
                                                    }
                                            }

                                            if (auto it =
    std::find(pCollB->GetCurrentColliders().begin(),
    pCollB->GetCurrentColliders().end(), pCollA); it !=
    pCollB->GetCurrentColliders().end())
                                            {
                                                    pCollB->GetCurrentColliders().erase(it);

                                                    if (pCollA->GetOwner() !=
    NULL)
                                                    {
                                                            EntityStoppedCollisionEvent
    event(pair.second, pCollA->GetOwner(), pCollA->GetTag());
                                                            pCollB->GetOwner()->OnEvent(event);
                                                    }
                                                    else
                                                    {
                                                            EntityStoppedCollisionEvent
    event(pair.second, nullptr, ""); pCollB->GetOwner()->OnEvent(event);
                                                    }
                                            }
                                    }
                            }
                    }
            }
    }*/
  }
};
} // namespace Lamp