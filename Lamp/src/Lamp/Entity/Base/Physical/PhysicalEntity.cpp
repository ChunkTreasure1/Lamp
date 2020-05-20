#include "lppch.h"
#include "PhysicalEntity.h"

#include "Lamp/Entity/Base/Entity.h"
#include "Lamp/Brushes/Brush.h"

namespace Lamp
{
	void PhysicalEntity::Integrate(float delta)
	{
		if (m_pEntity)
		{
			m_pEntity->SetPosition(m_pEntity->GetPosition() + m_Velocity * delta);
			glm::vec3 diff = m_pEntity->GetPosition() - m_LastPosition;

			if (m_Collider)
			{
				m_Collider->Transform(diff);
			}
			m_LastPosition = m_pEntity->GetPosition();
		}
	}
}