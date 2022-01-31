#include "lppch.h"
#include "Level.h"

#include "Lamp/Objects/Entity/BaseComponents/CameraComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/PointLightComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/DirectionalLightComponent.h"
#include "Lamp/Physics/Physics.h"
#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Lamp/Objects/Brushes/Brush.h"

#include "Lamp/GraphKey/NodeRegistry.h"

#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Lamp/Objects/Brushes/Brush.h"

#include "Lamp/World/Terrain.h"

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
			if (m_PointLights[i]->id == light->id)
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

	void RenderUtils::UnegisterMeshComponent(uint32_t id)
	{
		m_MeshComponents.erase(id);
	}

	Level::Level(const std::string& name)
		: m_name(name)
	{
		m_layers.reserve(100);
	
		m_layers.emplace_back("Main", 0, true);
	}

	Level::Level()
	{
		//Reserve 100 layer slots
		m_layers.reserve(100);
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
				m_renderUtils.RegisterPointLight(&lightComp->GetLight());
			}

			if (auto dirLightComp = pair.second->GetComponent<DirectionalLightComponent>())
			{
				m_renderUtils.RegisterDirectionalLight(&dirLightComp->GetLight());
			}
		}

		for (auto& brush : m_Brushes)
		{
			for (auto& layer : m_layers)
			{
				if (brush.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(brush.second);
				}
			}
		}

		for (const auto& layer : level.m_layers)
		{
			ObjectLayer l(layer.Name, layer.ID, layer.Active);
			m_layers.push_back(l);
		}

		for (auto& entity : m_Entities)
		{
			for (auto& layer : m_layers)
			{
				if (entity.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(entity.second);
				}
			}
		}

		for (auto& brush : m_Brushes)
		{
			for (auto& layer : m_layers)
			{
				if (brush.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(brush.second);
				}
			}
		}

		m_skybox = level.m_skybox;
		m_environment = level.m_environment;
		m_name = level.m_name;
	}

	Level::~Level()
	{

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

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EditorViewportSizeChangedEvent>(LP_BIND_EVENT_FN(Level::OnViewportResize));
	}

	void Level::UpdateEditor(Timestep ts)
	{
	}

	void Level::UpdateSimulation(Timestep ts)
	{
		Physics::GetScene()->Simulate(ts);
	}

	void Level::UpdateRuntime(Timestep ts)
	{
		AppUpdateEvent e(ts);
		OnEvent(e);

		Physics::GetScene()->Simulate(ts);
	}

	void Level::RenderEditor(const Ref<CameraBase> camera)
	{
		AppRenderEvent e(camera);
		OnEvent(e);

		RenderLevel(camera);
	}

	void Level::RenderSimulation(const Ref<CameraBase> camera)
	{
		AppRenderEvent e(camera);
		OnEvent(e);

		RenderLevel(camera);
	}

	void Level::RenderRuntime()
	{
		Ref<CameraBase> camera = nullptr;
		for (const auto& it : m_Entities)
		{
			if (const auto comp = it.second->GetComponent<CameraComponent>())
			{
				camera = comp->GetCamera();
				break;
			}
		}

		if (camera.get())
		{
			AppRenderEvent e(camera);
			OnEvent(e);

			RenderLevel(camera);
		}
	}

	void Level::Shutdown()
	{
		for (auto& ent : m_Entities)
		{
			delete ent.second;
			ent.second = nullptr;
		}

		for (auto& brush : m_Brushes)
		{
			delete brush.second;
			brush.second = nullptr;
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

		m_lastShowedGizmos = g_pEnv->shouldRenderGizmos;
		g_pEnv->shouldRenderGizmos = false;
	}

	void Level::OnRuntimeEnd()
	{
		Physics::DestroyScene();
		g_pEnv->shouldRenderGizmos = m_lastShowedGizmos;
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

	void Level::AddLayer(const ObjectLayer& layer)
	{
		m_layers.push_back(layer);
	}

	void Level::RemoveLayer(uint32_t id)
	{
		for (auto& layer : m_layers)
		{
			if (layer.ID == id)
			{
				for (auto& obj : layer.Objects)
				{
					obj->Destroy();
				}
			}
		}

		if (auto it = std::find_if(m_layers.begin(), m_layers.end(), [id](ObjectLayer& layer) { return id == layer.ID; }); it != m_layers.end())
		{
			m_layers.erase(it);
		}
	}

	void Level::MoveObjectToLayer(uint32_t currLayer, uint32_t newLayer, uint32_t objId)
	{
		ObjectLayer* currObjLayer;
		ObjectLayer* newObjLayer;

		for (auto& layer : m_layers)
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
		for (auto& layer : m_layers)
		{
			if (layer.ID == obj->GetLayerID())
			{
				layer.Objects.push_back(obj);
				return;
			}
		}
	}

	void Level::RemoveFromLayer(Object* obj)
	{
		for (auto& layer : m_layers)
		{
			if (auto it = std::find(layer.Objects.begin(), layer.Objects.end(), obj); it != layer.Objects.end())
			{
				layer.Objects.erase(it);
				break;
			}
		}
	}

	void Level::RenderLevel(const Ref<CameraBase> camera)
	{
		Renderer::Begin(camera);

		for (const auto& pass : m_renderPasses)
		{
			if (pass.graphicsPipeline)
			{
				Renderer::BeginPass(pass.graphicsPipeline);

				Renderer::DrawBuffer();

				Renderer::EndPass();
			}
			else
			{
				pass.computeExcuteCommand();
			}
		}

		Renderer::End();
	}

	bool Level::OnViewportResize(EditorViewportSizeChangedEvent& e)
	{
		return false;
	}
}