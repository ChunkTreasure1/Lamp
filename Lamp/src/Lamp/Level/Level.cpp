#include "lppch.h"
#include "Level.h"

#include "Lamp/Objects/Entity/BaseComponents/CameraComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/LightComponent.h"
#include "Lamp/Physics/Physics.h"

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
			if (auto lightComp = pair.second->GetComponent<LightComponent>())
			{
				m_RenderUtils.RegisterPointLight(lightComp->GetPointLight());
			}
		}
		m_Environment = level.m_Environment;
		m_Name = level.m_Name;
		m_Path = level.m_Path;
		m_Layers = level.m_Layers;
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
		RenderPassManager::Get()->RenderPasses(camera);
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
			RenderPassManager::Get()->RenderPasses(camera);
		}
	}
	void Level::OnRuntimeStart()
	{
		Physics::CreateScene();
		Physics::CreateActors(this);

		m_LastShowedGizmos = g_pEnv->ShouldRenderGizmos;
		g_pEnv->ShouldRenderGizmos = false;
	}

	void Level::OnRuntimeEnd()
	{
		Physics::DestroyScene();
		g_pEnv->ShouldRenderGizmos = m_LastShowedGizmos;
	}

	void Level::SetupLights()
	{
		for (auto& entity : m_Entities)
		{
			if (auto& comp = entity.second->GetComponent<LightComponent>())
			{
				m_RenderUtils.RegisterPointLight(comp->GetPointLight());
			}
		}
	}
}