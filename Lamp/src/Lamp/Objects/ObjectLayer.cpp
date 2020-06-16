#include "lppch.h"
#include "ObjectLayer.h"

namespace Lamp
{
	Ref<ObjectLayerManager> ObjectLayerManager::s_ObjectLayerManager = nullptr;
	
	void ObjectLayerManager::OnEvent(Event& e)
	{
		for (auto& l : m_Layers)
		{
			if (!l.IsActive)
			{
				continue;
			}

			for (int i = 0; i < l.Objects.size(); i++)
			{
				if (!l.Objects[i]->GetIsActive())
				{
					continue;
				}

				l.Objects[i]->OnEvent(e);
			}
		}
	}

	Object* ObjectLayerManager::GetObjectFromPoint(const glm::vec3& pos, const glm::vec3& origin)
	{
		Ray ray;
		ray.origin = origin;
		ray.direction = pos;

		for (auto& layer : m_Layers)
		{
			for (auto& obj : layer.Objects)
			{
				if (obj->GetPhysicalEntity()->GetCollider()->IntersectRay(ray).IsIntersecting)
				{
					return obj;
				}
			}
		}

		return nullptr;
	}
}