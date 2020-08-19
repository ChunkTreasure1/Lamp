#pragma once

#include "Object.h"
#include "Lamp/Event/Event.h"

#include <string>

namespace Lamp
{
	struct ObjectLayer
	{
		ObjectLayer(uint32_t num, const std::string& name, bool state)
			: ID(num), Name(name), IsDestroyable(state), IsActive(true)
		{}

		ObjectLayer(uint32_t num)
			: ID(num), Name("Layer"), IsDestroyable(true)
		{}

		uint32_t ID;
		std::string Name;
		std::vector<Object*> Objects;
		bool IsDestroyable;
		bool IsActive;
	};

	class ObjectLayerManager
	{
	public:
		ObjectLayerManager()
		{}

		void OnEvent(Event& e);

		inline void AddLayer(ObjectLayer& layer) { m_Layers.push_back(layer); }
		inline bool RemoveLayer(uint32_t id)
		{
			for (int i = 0; i < m_Layers.size(); i++)
			{
				if (m_Layers[i].ID == id)
				{
					m_Layers.erase(m_Layers.begin() + i);
				}
			}
		}
		inline std::vector<ObjectLayer>& GetLayers() { return m_Layers; }
		inline void SetLayers(std::vector<ObjectLayer>& layers) { m_Layers = layers; }

		void AddToLayer(Object* obj, uint32_t layerId)
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
		void AddToLayer(Object* obj, const std::string& name)
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

		void RemoveFromLayer(Object* obj, uint32_t layerId)
		{
			for (auto layer : m_Layers)
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

		void MoveToLayer(Object* obj, uint32_t layerId)
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

		Object* GetObjectFromPoint(const glm::vec3& pos, const glm::vec3& origin);

	public:
		static void SetCurrentManager(Ref<ObjectLayerManager>& manager) { s_ObjectLayerManager = manager; }
		static Ref<ObjectLayerManager>& Get() { return s_ObjectLayerManager; }
	private:
		bool Exists(uint32_t layerId)
		{
			for (auto& layer : m_Layers)
			{
				if (layer.ID == layerId)
				{
					return true;
				}
			}

			return false;
		}

	private:
		static Ref<ObjectLayerManager> s_ObjectLayerManager;

	private:
		std::vector<ObjectLayer> m_Layers;
	};
}