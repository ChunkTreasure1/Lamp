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

	void ObjectLayerManager::Destroy()
	{
		m_Layers.clear();
	}

	inline bool ObjectLayerManager::RemoveLayer(uint32_t id)
	{
		for (int i = 0; i < m_Layers.size(); i++)
		{
			if (m_Layers[i].ID == id)
			{
				m_Layers.erase(m_Layers.begin() + i);
			}
		}
	}

	void ObjectLayerManager::AddToLayer(Object* obj, uint32_t layerId)
	{
		for (int i = 0; i < m_Layers.size(); i++)
		{
			if (m_Layers[i].ID == layerId)
			{
				m_Layers[i].Objects.push_back(obj);
				return;
			}
		}

		//Layer not found, set it to main layer
		m_Layers[0].Objects.push_back(obj);
	}

	void ObjectLayerManager::AddToLayer(Object* obj, const std::string& name)
	{
		for (int i = 0; i < m_Layers.size(); i++)
		{
			if (m_Layers[i].Name == name)
			{
				m_Layers[i].Objects.push_back(obj);
			}
		}

		//Layer not found, set it to main layer
		m_Layers[0].Objects.push_back(obj);
	}

	void ObjectLayerManager::RemoveFromLayer(Object* obj, uint32_t layerId)
	{
		for (auto& layer : m_Layers)
		{
			if (layer.ID == layerId)
			{
				auto it = std::find(layer.Objects.begin(), layer.Objects.end(), obj);
				if (it != layer.Objects.end())
				{
					layer.Objects.erase(it);
				}

				delete obj;
				obj = nullptr;
			}
		}
	}

	bool ObjectLayerManager::Remove(Object* obj)
	{
		for (auto& layer : m_Layers)
		{
			auto it = std::find(layer.Objects.begin(), layer.Objects.end(), obj);
			if (it != layer.Objects.end())
			{
				layer.Objects.erase(it);
				return true;
			}
		}

		return false;
	}

	void ObjectLayerManager::MoveToLayer(Object* obj, uint32_t layerId)
	{
		if (!Exists(layerId))
		{
			LP_CORE_WARN("Layer does not exist!");
			return;
		}

		for (auto& layer : m_Layers)
		{
			if (layer.ID == obj->GetLayerID())
			{
				auto it = std::find(layer.Objects.begin(), layer.Objects.end(), obj);
				if (it != layer.Objects.end())
				{
					layer.Objects.erase(it);
				}
			}
		}

		for (auto& layer : m_Layers)
		{
			if (layer.ID == layerId)
			{
				layer.Objects.push_back(obj);
				obj->SetLayerID(layerId);

				return;
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