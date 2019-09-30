#include "Entity.h"

namespace LampEntity
{
	void IEntity::Update()
	{
		for (auto& comp : m_pComponents)
		{
			comp->Update();
		}
	}
}