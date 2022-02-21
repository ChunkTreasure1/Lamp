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
#include "Platform/Vulkan/VulkanRenderer.h"

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

		m_environment = level.m_environment;
		m_name = level.m_name;

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
	}

	void Level::UpdateSimulation(Timestep ts)
	{
		Physics::GetScene()->Simulate(ts);
		
		AppRenderEvent e(nullptr);
		OnEvent(e);
	}

	void Level::UpdateRuntime(Timestep ts, const Ref<CameraBase> renderCamera)
	{
		AppUpdateEvent e(ts);
		OnEvent(e);

		AppRenderEvent renderE(renderCamera);
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
		Ref<CameraBase> camera = nullptr;
		for (const auto& it : m_entities)
		{
			if (const auto comp = it.second->GetComponent<CameraComponent>())
			{
				camera = comp->GetCamera();
				break;
			}
		}

		if (camera.get())
		{
			RenderLevel();
		}
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

	void Level::SetupRenderPasses()
	{
		//Depth PrePass
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::R32F,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("depthPrePass");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.debugName = "Pre Depth";
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipeline::Create(pipelineSpec);
		}

		//Light culling
		{
			auto& pass = m_renderPasses.emplace_back();
			auto [pipeline, command] = Renderer::Get().CreateLightCullingPipeline(m_renderPasses[m_renderPasses.size() - 2].graphicsPipeline->GetSpecification().framebuffer->GetDepthAttachment());
			pass.computePipeline = pipeline;
			pass.computeExcuteCommand = command;
		}

		//Skybox
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::R32UI,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.isSwapchain = false;
			pipelineSpec.depthWrite = false;
			pipelineSpec.cullMode = CullMode::Back;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.drawType = DrawType::Skybox;
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			m_geometryFramebuffer = pipelineSpec.framebuffer;

			pipelineSpec.shader = ShaderLibrary::GetShader("skybox");
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" }
			};

			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipeline::Create(pipelineSpec);
		}

		//Terrain
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.existingImages =
			{
				{ 0, m_geometryFramebuffer->GetColorAttachment(0) },
				{ 1, m_geometryFramebuffer->GetColorAttachment(1) },
				{ 2, m_geometryFramebuffer->GetDepthAttachment() }
			};
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::R32UI,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.isSwapchain = false;
			pipelineSpec.cullMode = CullMode::Front;
			pipelineSpec.topology = Topology::PatchList;
			pipelineSpec.drawType = DrawType::Terrain;
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.shader = ShaderLibrary::GetShader("terrain");
			pipelineSpec.useTessellation = true;

			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" }
			};

			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipeline::Create(pipelineSpec);
		}

		//Geometry pass
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.existingImages =
			{
				{ 0, m_geometryFramebuffer->GetColorAttachment(0) },
				{ 1, m_geometryFramebuffer->GetColorAttachment(1) },
				{ 2, m_geometryFramebuffer->GetDepthAttachment() }
			};
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA32F,
				ImageFormat::R32UI,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("pbrForward");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.shaderStorageBufferSets = Renderer::Get().GetStorage().shaderStorageBufferSet;
			pipelineSpec.debugName = "Geometry";
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			pipelineSpec.framebufferInputs =
			{
				{ Renderer::Get().GetDefaults().brdfFramebuffer->GetColorAttachment(0), 0, 7 }
			};

			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipeline::Create(pipelineSpec);
		}

		//Quad
		{
			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = Renderer2D::Get().CreateQuadPipeline(m_geometryFramebuffer);
		}

		//Line
		{
			//auto& pass = m_renderPasses.emplace_back();
			//pass.graphicsPipeline = Renderer2D::Get().CreateLinePipeline(m_geometryFramebuffer);
		}

		//Translucency pass
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.clearColor = { 0.f, 0.f, 0.f, 0.f };
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA16F,
				ImageFormat::RGBA16F,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);

			pipelineSpec.shader = ShaderLibrary::GetShader("pbrSSS");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.drawType = DrawType::Translucency;
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.shaderStorageBufferSets = Renderer::Get().GetStorage().shaderStorageBufferSet;
			pipelineSpec.debugName = "Translucency";
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			pipelineSpec.framebufferInputs =
			{
				{ Renderer::Get().GetDefaults().brdfFramebuffer->GetColorAttachment(0), 0, 7 }
			};

			//auto& pass = m_renderPasses.emplace_back();
			//pass.graphicsPipeline = RenderPipeline::Create(pipelineSpec);
		}

		//Composite
		{
			FramebufferSpecification framebufferSpec{};
			framebufferSpec.swapchainTarget = false;
			framebufferSpec.attachments =
			{
				ImageFormat::RGBA,
				ImageFormat::DEPTH32F
			};

			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
			pipelineSpec.shader = ShaderLibrary::GetShader("composite");
			pipelineSpec.isSwapchain = false;
			pipelineSpec.topology = Topology::TriangleList;
			pipelineSpec.drawType = DrawType::ScreenQuad;
			pipelineSpec.uniformBufferSets = Renderer::Get().GetStorage().uniformBufferSet;
			pipelineSpec.vertexLayout =
			{
				{ ElementType::Float3, "a_Position" },
				{ ElementType::Float3, "a_Normal" },
				{ ElementType::Float3, "a_Tangent" },
				{ ElementType::Float3, "a_Bitangent" },
				{ ElementType::Float2, "a_TexCoords" },
			};

			pipelineSpec.framebufferInputs =
			{
				{ m_geometryFramebuffer->GetColorAttachment(0), 0, 5 },
			};

			auto& pass = m_renderPasses.emplace_back();
			pass.graphicsPipeline = RenderPipeline::Create(pipelineSpec);
		}
	}

	void Level::RenderLevel()
	{
		LP_PROFILE_FUNCTION();

		//3D passes
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