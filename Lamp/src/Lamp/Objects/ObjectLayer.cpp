#include "lppch.h"
#include "ObjectLayer.h"

#include "Lamp/Physics/PhysicsEngine.h"

namespace Lamp
{
	ObjectLayerManager* ObjectLayerManager::s_ObjectLayerManager = nullptr;
	
	ObjectLayerManager::ObjectLayerManager()
	{
		s_ObjectLayerManager = this;
	
		ObjectLayer layer(0, "Main", false);
		AddLayer(layer);
	}

	void ObjectLayerManager::OnEvent(Event& e)
	{
		auto layerFunc = [&e](ObjectLayer& layer)
		{
			if (!layer.IsActive)
			{
				return;
			}

			for (auto& obj : layer.Objects)
			{
				if (e.GetEventType() & obj->GetEventMask())
				{
					if (!obj->GetIsActive())
					{
						continue;
					}

					obj->OnEvent(e);
				}
				else
				{
					continue;
				}
			}
		};

		std::for_each(std::execution::par_unseq, m_Layers.begin(), m_Layers.end(), layerFunc);
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
		auto it = std::find_if(m_Layers.begin(), m_Layers.end(), [&layerId](const ObjectLayer& layer) { return layer.ID == layerId; });

		if (it != m_Layers.end())
		{
			it->Objects.push_back(obj);
			return;
		}

		//Layer not found, set it to main layer
		m_Layers[0].Objects.push_back(obj);
	}

	void ObjectLayerManager::AddToLayer(Object* obj, const std::string& name)
	{
		auto it = std::find_if(m_Layers.begin(), m_Layers.end(), [&name](const ObjectLayer& layer) { return layer.Name == name; });

		if (it != m_Layers.end())
		{
			it->Objects.push_back(obj);
			return;
		}

		//Layer not found, set it to main layer
		m_Layers[0].Objects.push_back(obj);
	}

	void ObjectLayerManager::RemoveFromLayer(Object* obj, uint32_t layerId)
	{
		auto it = std::find_if(m_Layers.begin(), m_Layers.end(), [&layerId](const ObjectLayer& layer) { return layer.ID == layerId; });

		if (it != m_Layers.end())
		{
			auto ob = std::find(it->Objects.begin(), it->Objects.end(), obj);
			if (ob != it->Objects.end())
			{
				it->Objects.erase(ob);
			}

			delete obj;
			obj = nullptr;
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

	Object* ObjectLayerManager::GetObjectFromPoint(const glm::vec3& origin, const glm::vec3& dir)
	{
			Ray r;
		r.dir = dir;
		r.origin = origin;

		for (auto& layer : m_Layers)
		{
			for (auto& obj : layer.Objects)
			{
			}
		}

		return nullptr;
	}

	Object* ObjectLayerManager::GetObjectFromId(uint32_t id)
	{
		for (auto& layer : m_Layers)
		{
			for (auto& obj : layer.Objects)
			{
				if (obj->GetID() == id)
				{
					return obj;
				}
			}
		}
		return nullptr;
	}
}