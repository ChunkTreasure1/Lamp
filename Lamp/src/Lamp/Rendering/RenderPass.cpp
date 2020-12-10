#include "lppch.h"
#include "RenderPass.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Core/Application.h"

namespace Lamp
{
	Ref<RenderPassManager> RenderPassManager::s_Instance = nullptr;

	RenderPass::RenderPass(Ref<FrameBuffer>& frameBuffer, const RenderPassInfo& passInfo, std::vector<RenderFunc> extraRenders)
		: m_FrameBuffer(frameBuffer), m_PassInfo(passInfo), m_ExtraRenders(extraRenders)
	{
	}

	void RenderPass::Render()
	{
		if (m_PassInfo.IsShadowPass)
		{
			m_PassInfo.ViewProjection = g_pEnv->DirLight.ViewProjection;
			m_PassInfo.LightViewProjection = g_pEnv->DirLight.ViewProjection;
		}
		else
		{
			m_PassInfo.ViewProjection = m_PassInfo.Camera->GetViewProjectionMatrix();
			m_PassInfo.LightViewProjection = g_pEnv->DirLight.ViewProjection;
		}

		RenderCommand::SetClearColor(m_PassInfo.ClearColor);
		RenderCommand::Clear();

		m_FrameBuffer->Bind();
		RenderCommand::Clear();

		Renderer3D::Begin(m_PassInfo);

		AppRenderEvent renderEvent(m_PassInfo);
		ObjectLayerManager::Get()->OnEvent(renderEvent);
		Application::Get().OnEvent(renderEvent);

		for (auto& f : m_ExtraRenders)
		{
			f();
		}

		Renderer3D::DrawSkybox();
		Renderer3D::End();
		m_FrameBuffer->Unbind();
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