#include "lppch.h"
#include "Level.h"

#include "Lamp/Objects/Entity/BaseComponents/CameraComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/PointLightComponent.h"
#include "Lamp/Objects/Entity/BaseComponents/DirectionalLightComponent.h"
#include "Lamp/Objects/Entity/Entity.h"
#include "Lamp/Objects/Brushes/Brush.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/RenderPipelineLibrary.h"

#include "Lamp/Physics/Physics.h"
#include "Lamp/GraphKey/NodeRegistry.h"
#include "Lamp/World/Terrain.h"

namespace Lamp
{
	Level::Level(const std::string& name)
		: m_name(name)
	{
		m_layers.reserve(100);
		m_layers.emplace_back("Main", 0, true);
		SetupRenderPasses();
	}

	Level::Level()
	{
		//Reserve 100 layer slots
		m_layers.reserve(100);
		SetupRenderPasses();
	}

	Level::Level(const Level& level)
	{
		m_environment = level.m_environment;
		m_name = level.m_name;

		m_environment.m_directionalLights.clear();
		m_environment.m_pointLights.clear();

		for (auto& brush : level.m_brushes)
		{
			m_brushes.emplace(std::make_pair(brush.first, Brush::Duplicate(brush.second, false)));
		}

		for (auto& entity : level.m_entities)
		{
			std::pair pair = std::make_pair(entity.first, Entity::Duplicate(entity.second, false));
			m_entities.emplace(pair);
			if (auto lightComp = pair.second->GetComponent<PointLightComponent>())
			{
				m_environment.RegisterPointLight(&lightComp->GetLight());
			}

			if (auto dirLightComp = pair.second->GetComponent<DirectionalLightComponent>())
			{
				m_environment.RegisterDirectionalLight(&dirLightComp->GetLight());
			}
		}

		for (auto& brush : m_brushes)
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

		for (auto& entity : m_entities)
		{
			for (auto& layer : m_layers)
			{
				if (entity.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(entity.second);
				}
			}
		}

		for (auto& brush : m_brushes)
		{
			for (auto& layer : m_layers)
			{
				if (brush.second->GetLayerID() == layer.ID)
				{
					layer.Objects.push_back(brush.second);
				}
			}
		}

		SetupRenderPasses();
	}

	Level::~Level()
	{

	}

	void Level::OnEvent(Event& e)
	{
		for (auto& it : m_entities)
		{
			it.second->OnEvent(e);
		}

		for (auto& it : m_brushes)
		{
			it.second->OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EditorViewportSizeChangedEvent>(LP_BIND_EVENT_FN(Level::OnViewportResize));
	}

	void Level::UpdateEditor(Timestep ts, const Ref<CameraBase> renderCamera)
	{
		AppRenderEvent e(renderCamera);
		OnEvent(e);

		myRuntimeCamera = renderCamera;
	}

	void Level::UpdateSimulation(Timestep ts, const Ref<CameraBase> renderCamera)
	{
		Physics::GetScene()->Simulate(ts);

		AppRenderEvent e(myRuntimeCamera);
		OnEvent(e);

		myRuntimeCamera = renderCamera;
	}

	void Level::UpdateRuntime(Timestep ts)
	{
		AppUpdateEvent e(ts);
		OnEvent(e);

		AppRenderEvent renderE(myRuntimeCamera);
		OnEvent(renderE);

		Physics::GetScene()->Simulate(ts);
	}

	void Level::RenderEditor()
	{
		RenderLevel();
	}

	void Level::RenderSimulation()
	{
		RenderLevel();
	}

	void Level::RenderRuntime()
	{
		RenderLevel();
	}

	void Level::Shutdown()
	{
		for (auto& ent : m_entities)
		{
			delete ent.second;
			ent.second = nullptr;
		}

		for (auto& brush : m_brushes)
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

		for (const auto& entity : m_entities)
		{
			if (entity.second->HasComponent<CameraComponent>())
			{
				myRuntimeCamera = entity.second->GetComponent<CameraComponent>()->GetCamera();
				break;
			}
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
		ObjectLayer* currObjLayer = nullptr;
		ObjectLayer* newObjLayer = nullptr;

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

		Object* obj = nullptr;
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

	void Level::SetupRenderPasses()
	{
		//Skybox
		{
			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipelineLibrary::GetPipeline("Skybox");
		}

		//GBuffer
		{
			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipelineLibrary::GetPipeline("Deferred");
			m_geometryFramebuffer = pass.graphicsPipeline->GetSpecification().framebuffer;
		}

		//Shading
		{
			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipelineLibrary::GetPipeline("Shading");
		}
	}

	void Level::RenderLevel()
	{
		LP_PROFILE_FUNCTION();

		if (!myRuntimeCamera)
		{
			return;
		}

		RenderCommand::Begin(myRuntimeCamera);

		for (const auto& pass : m_renderPasses)
		{
			if (pass.graphicsPipeline)
			{
				RenderCommand::BeginPass(pass.graphicsPipeline);
				RenderCommand::DispatchRenderCommands();
				RenderCommand::EndPass();
			}
			else
			{
				pass.computeExcuteCommand();
			}
		}

		RenderCommand::End();
	}

	bool Level::OnViewportResize(EditorViewportSizeChangedEvent& e)
	{
		for (const auto& pass : m_renderPasses)
		{
			if (pass.graphicsPipeline)
			{
				pass.graphicsPipeline->GetSpecification().framebuffer->Resize(e.GetWidth(), e.GetHeight());
			}
		}

		return false;
	}
}