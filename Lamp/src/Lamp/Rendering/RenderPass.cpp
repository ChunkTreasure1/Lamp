#include "lppch.h"
#include "RenderPass.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Core/Application.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"

namespace Lamp
{
	Ref<RenderPassManager> RenderPassManager::s_Instance = nullptr;

	RenderPass::RenderPass(const RenderPassSpecification& spec)
		: m_PassSpec(spec)
	{
	}

	void RenderPass::Render()
	{
		LP_PROFILE_SCOPE("RenderPass::Render::" + m_PassSpec.Name);

		if (m_PassSpec.type == PassType::PointShadow)
		{
			m_PassSpec.LightIndex = 0;
			for (auto& light : g_pEnv->pRenderUtils->GetPointLights())
			{
				light->ShadowBuffer->Bind();
				RenderCommand::Clear();

				Renderer3D::Begin(m_PassSpec);

				AppRenderEvent renderEvent(m_PassSpec);
				ObjectLayerManager::Get()->OnEvent(renderEvent);
				Application::Get().OnEvent(renderEvent);

				Renderer3D::End();
				light->ShadowBuffer->Unbind();
				m_PassSpec.LightIndex++;
			}
		}
		else
		{
			RenderCommand::SetClearColor(m_PassSpec.TargetFramebuffer->GetSpecification().ClearColor);
			RenderCommand::Clear();

			m_PassSpec.TargetFramebuffer->Bind();
			RenderCommand::Clear();

			Renderer3D::Begin(m_PassSpec);

			AppRenderEvent renderEvent(m_PassSpec);
			ObjectLayerManager::Get()->OnEvent(renderEvent);
			Application::Get().OnEvent(renderEvent);

			for (auto& f : m_PassSpec.ExtraRenders)
			{
				f();
			}

			Renderer3D::End();
			m_PassSpec.TargetFramebuffer->Unbind();
		}
	}

	void RenderPassManager::AddPass(Ref<RenderPass>& pass)
	{
		pass->SetID((uint32_t)m_RenderPasses.size());
		m_RenderPasses.push_back(pass);
	}

	bool RenderPassManager::RemovePass(uint32_t id)
	{
		auto it = std::remove_if(m_RenderPasses.begin(), m_RenderPasses.end(), [id](auto pass) { return pass->GetID() == id; });
		m_RenderPasses.erase(it);

		return it != m_RenderPasses.end();
	}

	void RenderPassManager::RenderPasses()
	{
		for (auto& pass : m_RenderPasses)
		{
			pass->Render();
		}
	}
}