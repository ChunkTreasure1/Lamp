#include "lppch.h"
#include "PhysicalEntity.h"

#include "Lamp/Entity/Base/Entity.h"

namespace Lamp
{
	void PhysicalEntity::Integrate(float delta)
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() + m_Velocity * delta);
	}
}