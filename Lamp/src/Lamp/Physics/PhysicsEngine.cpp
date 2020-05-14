#include "lppch.h"
#include "PhysicsEngine.h"

#include "Colliders/BoundingSphere.h"

namespace Lamp
{
	Ref<PhysicsEngine> PhysicsEngine::s_PhysicsEngine = std::make_shared<PhysicsEngine>();

	void PhysicsEngine::AddEntity(Ref<PhysicalEntity>& entity)
	{
		m_PhysicalEntites.push_back(entity);
	}

	void PhysicsEngine::Simulate(float delta)
	{
		for (auto& entity : m_PhysicalEntites)
		{
			entity->Integrate(delta);
		}
	}

	void PhysicsEngine::HandleCollisions()
	{
		for (size_t i = 0; i < m_PhysicalEntites.size(); i++)
		{
			for (size_t j = i + 1; j < m_PhysicalEntites.size(); j++)
			{
				IntersectData data = m_PhysicalEntites[i]->GetCollider()->Intersect(m_PhysicalEntites[j]->GetCollider());

				if (data.IsIntersecting)
				{
					glm::vec3 dir = glm::normalize(data.Direction);
					glm::vec3 otherDir = glm::reflect(dir, glm::normalize(m_PhysicalEntites[i]->GetVelocity()));

					m_PhysicalEntites[i]->SetVelocity(glm::reflect(glm::normalize(m_PhysicalEntites[i]->GetVelocity()), otherDir));
					m_PhysicalEntites[j]->SetVelocity(glm::reflect(glm::normalize(m_PhysicalEntites[j]->GetVelocity()), dir));
				}
			}
		}
	}
}