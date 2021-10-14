#include "lppch.h"
#include "Level.h"

#include "Lamp/Objects/Entity/BaseComponents/CameraComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/PointLightComponent.h"
#include "Lamp/Physics/Physics.h"
#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Lamp/Objects/Brushes/Brush.h"

#include "Lamp/GraphKey/NodeRegistry.h"
#include "Lamp/Rendering/RenderGraph/RenderGraph.h"

namespace Lamp
{
	RenderUtils::~RenderUtils()
	{
		m_PointLights.clear();
	}

	void RenderUtils::RegisterPointLight(Lamp::PointLight* light)
	{
		m_PointLights.push_back(light);
	}

	bool RenderUtils::UnregisterPointLight(Lamp::PointLight* light)
	{
		for (int i = 0; i < m_PointLights.size(); i++)
		{
			if (m_PointLights[i]->Id == light->Id)
			{
				m_PointLights.erase(m_PointLights.begin() + i);
				return true;
			}
		}

		return false;
	}

	void RenderUtils::RegisterDirectionalLight(DirectionalLight* light)
	{
		m_DirectionalLights.push_back(light);
	}

	bool RenderUtils::UnregisterDirectionalLight(DirectionalLight* light)
	{
		for (int i = 0; i < m_DirectionalLights.size(); i++)
		{
			if (m_DirectionalLights[i]->Id == light->Id)
			{
				m_DirectionalLights.erase(m_DirectionalLights.begin() + i);
				return true;
			}
		}

		return false;
	}

	void RenderUtils::RegisterMeshComponent(uint32_t id, Lamp::MeshComponent* mesh)
	{
		m_MeshComponents.insert(std::make_pair(id, mesh));
	}

	bool RenderUtils::UnegisterMeshComponent(uint32_t id)
	{
		if (m_MeshComponents.find(id) != m_MeshComponents.end())
		{
			m_MeshComponents.erase(id);
			return true;
		}
		return false;
	}

	Level::Level(const Level& level)
	{
		for (auto& brush : level.m_Brushes)
		{
			m_Brushes.emplace(std::make_pair(brush.first, Brush::Duplicate(brush.second, false)));
		}

		for (auto& entity : level.m_Entities)
		{
			std::pair pair = std::make_pair(entity.first, Entity::Duplicate(entity.second, false));
			m_Entities.emplace(pair);
			if (auto lightComp = pair.second->GetComponent<PointLightComponent>())
			{
				m_RenderUtils.RegisterPointLight(lightComp->GetLight());
			}
		}

		for (auto& brush : m_Brushes)
		{
			for (auto& layer : m_Layers)
			{
				if (brush.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(brush.second);
				}
			}
		}

		for (auto layer : level.m_Layers)
		{
			ObjectLayer l(layer.Name, layer.ID, layer.Active);
			m_Layers.push_back(l);
		}

		for (auto& entity : m_Entities)
		{
			for (auto& layer : m_Layers)
			{
				if (entity.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(entity.second);
				}
			}
		}

		for (auto& brush : m_Brushes)
		{
			for (auto& layer : m_Layers)
			{
				if (brush.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(brush.second);
				}
			}
		}

		m_Environment = level.m_Environment;
		m_Name = level.m_Name;
	}

	void Level::OnEvent(Event& e)
	{
		for (auto& it : m_Entities)
		{
			it.second->OnEvent(e);
		}

		for (auto& it : m_Brushes)
		{
			it.second->OnEvent(e);
		}
	}

	void Level::UpdateEditor(Timestep ts, Ref<CameraBase>& camera)
	{
		AppRenderEvent e(camera);
		OnEvent(e);

		RenderLevel(camera);
	}
	void Level::UpdateSimulation(Timestep ts, Ref<CameraBase>& camera)
	{
		Physics::GetScene()->Simulate(ts);

		AppRenderEvent e(camera);
		OnEvent(e);

		RenderLevel(camera);
	}

	void Level::UpdateRuntime(Timestep ts)
	{
		AppUpdateEvent e(ts);
		OnEvent(e);

		Physics::GetScene()->Simulate(ts);

		Ref<CameraBase> camera;
		for (auto& it : m_Entities)
		{
			if (auto& comp = it.second->GetComponent<CameraComponent>())
			{
				camera = comp->GetCamera();
				break;
			}
		}

		if (camera)
		{
			AppRenderEvent e(camera);
			OnEvent(e);

			RenderLevel(camera);
		}
	}

	void Level::OnRuntimeStart()
	{
		Physics::CreateScene();
		Physics::CreateActors(this);

		for (const auto& node : NodeRegistry::s_StartNodes())
		{
			node->ActivateOutput(0);
		}

		m_LastShowedGizmos = g_pEnv->ShouldRenderGizmos;
		g_pEnv->ShouldRenderGizmos = false;
	}
	void Level::OnRuntimeEnd()
	{
		Physics::DestroyScene();
		g_pEnv->ShouldRenderGizmos = m_LastShowedGizmos;
	}

	void Level::OnSimulationStart()
	{
		Physics::CreateScene();
		Physics::CreateActors(this);
	}
	void Level::OnSimulationEnd()
	{
		Physics::DestroyScene();
	}

	void Level::SetupLights()
	{
		for (auto& entity : m_Entities)
		{
			if (auto& comp = entity.second->GetComponent<PointLightComponent>())
			{
				m_RenderUtils.RegisterPointLight(comp->GetLight());
			}
		}


	}

	void Level::AddLayer(const ObjectLayer& layer)
	{
		m_Layers.push_back(layer);
	}

	void Level::RemoveLayer(uint32_t id)
	{
		for (auto& layer : m_Layers)
		{
			if (layer.ID == id)
			{
				for (auto& obj : layer.Objects)
				{
					obj->Destroy();
				}
			}
		}

		if (auto it = std::find_if(m_Layers.begin(), m_Layers.end(), [id](ObjectLayer& layer) { return id == layer.ID; }); it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
	}

	void Level::MoveObjectToLayer(uint32_t currLayer, uint32_t newLayer, uint32_t objId)
	{
		ObjectLayer* currObjLayer;
		ObjectLayer* newObjLayer;

		for (auto& layer : m_Layers)
		{
			if (layer.ID == currLayer)
			{
				currObjLayer = &layer;
			}

			if (layer.ID == newLayer)
			{
				newObjLayer = &layer;
			}
		}

		Object* obj;
		for (auto& objects : currObjLayer->Objects)
		{
			if (objects->GetID() == objId)
			{
				obj = objects;
			}
		}

		if (!obj || !currObjLayer || !newObjLayer)
		{
			return;
		}

		if (auto it = std::find(currObjLayer->Objects.begin(), currObjLayer->Objects.end(), obj); it != currObjLayer->Objects.end())
		{
			currObjLayer->Objects.erase(it);
		}

		obj->SetLayerID(newLayer);
		newObjLayer->Objects.push_back(obj);
	}

	void Level::AddToLayer(Object* obj)
	{
		for (auto& layer : m_Layers)
		{
			if (layer.ID == obj->GetLayerID())
			{
				layer.Objects.push_back(obj);
				return;
			}
		}
	}

	void Level::RenderLevel(Ref<CameraBase> camera)
	{
		if (const auto& graph = Renderer3D::GetSettings().RenderGraph)
		{
			graph->Run(camera);
		}
		else
		{
			RenderPassManager::Get()->RenderPasses(camera);
		}
	}
}