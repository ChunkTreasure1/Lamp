#include "lppch.h"
#include "PhysicalEntity.h"

#include "Lamp/Entity/Base/Entity.h"
#include "Lamp/Brushes/Brush.h"
#include "Lamp/Entity/Base/EntityManager.h"

namespace Lamp
{
	void PhysicalEntity::Integrate(float delta)
	{
		if (!m_IsPhysicalized)
		{
			return;
		}

		Entity* pEnt = EntityManager::Get()->GetEntityFromPhysicalEntity(this);

		if (pEnt)
		{
			pEnt->SetPosition(pEnt->GetPosition() + m_Velocity * delta);
			glm::vec3 diff = pEnt->GetPosition() - m_LastPosition;

			if (m_Collider)
			{
				m_Collider->Transform(diff);
			}
			m_LastPosition = pEnt->GetPosition();
		}
	}
}