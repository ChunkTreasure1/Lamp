#include "lppch.h"
#include "RenderPass.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Core/Application.h"

namespace Lamp
{
	Ref<RenderPassManager> RenderPassManager::s_Instance = nullptr;

	RenderPass::RenderPass(Ref<FrameBuffer>& frameBuffer, const RenderPassInfo& passInfo, std::initializer_list<std::function<void()>> extraRenders)
		: m_FrameBuffer(frameBuffer), m_PassInfo(passInfo), m_ExtraRenders(extraRenders)
	{
	}

	void RenderPass::Render()
	{
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
			f;
		}

		Renderer3D::End();
		m_FrameBuffer->Unbind();
	}

	void RenderPassManager::AddPass(Ref<RenderPass>& pass)
	{
		pass->SetID(m_RenderPasses.size());
		m_RenderPasses.push_back(pass);
	}

	bool RenderPassManager::RemovePass(uint32_t id)
	{
		for (auto& pass : m_RenderPasses)
		{
			if (pass->GetID() == id)
			{
				auto it = std::find(m_RenderPasses.begin(), m_RenderPasses.end(), pass);
				m_RenderPasses.erase(it);

				return true;
			}
		}

		return false;
	}

	void RenderPassManager::RenderPasses()
	{
		for (auto& pass : m_RenderPasses)
		{
			pass->Render();
		}
	}
}