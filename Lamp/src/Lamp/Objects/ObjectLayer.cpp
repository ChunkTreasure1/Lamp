#include "lppch.h"
#include "ObjectLayer.h"

#include "Lamp/Physics/PhysicsEngine.h"

namespace Lamp
{
	Ref<ObjectLayerManager> ObjectLayerManager::s_ObjectLayerManager = nullptr;
	
	void ObjectLayerManager::OnEvent(Event& e)
	{
		for (auto it = m_Layers.begin(); it != m_Layers.end(); it++)
		{
			if (!it->IsActive)
			{
				continue;
			}

			for (auto& obj : it->Objects)
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

	Object* ObjectLayerManager::GetObjectFromPoint(const glm::vec2& mousePos, const glm::vec2& screenSize, Ref<PerspectiveCameraController>& camera)
	{
		glm::vec3 dir;
		glm::vec3 origin;

		PhysicsEngine::Get()->ScreenPosToWorldRay(mousePos, screenSize, camera->GetCamera()->GetViewMatrix(), camera->GetCamera()->GetProjectionMatrix(), origin, dir);

		return PhysicsEngine::Get()->RaycastRef(origin, dir, 10000.f);
	}
}