#include "lppch.h"
#include "RenderPass.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Core/Application.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Level/Level.h"
#include "Lamp/Rendering/Renderer2D.h"

namespace Lamp
{
	Ref<RenderPassManager> RenderPassManager::s_Instance = nullptr;

	RenderPass::RenderPass(const RenderPassSpecification& spec)
		: m_passSpecification(spec)
	{
	}

	void RenderPass::Render(Ref<CameraBase>& camera)
	{
		LP_PROFILE_SCOPE("RenderPass::Render::" + m_passSpecification.name);

		RenderCommand::SetClearColor(m_passSpecification.targetFramebuffer->GetSpecification().ClearColor);
		m_passSpecification.targetFramebuffer->Bind();

		if (m_passSpecification.enableBlending)
		{
			RenderCommand::EnableBlending(true);
		}

		//Clear color if i should
		switch (m_passSpecification.clearType)
		{
			case ClearType::None:
				break;

			case ClearType::Color:
				RenderCommand::ClearColor();
				break;

			case ClearType::Depth:
				RenderCommand::ClearDepth();
				break;

			case ClearType::ColorDepth:
				RenderCommand::Clear();
				break;
		}

		Renderer3D::BeginPass(m_passSpecification);

		switch (m_passSpecification.drawType)
		{
			case DrawType::All:
				Renderer3D::DrawRenderBuffer();
				break;

			case DrawType::Forward:
				Renderer3D::DrawRenderBuffer();
				break;

			case DrawType::Line:
				break;

			case DrawType::Quad:
				Renderer3D::RenderQuad();
				break;

			default:
				break;
		}

		if (m_passSpecification.draw2D)
		{
			Renderer2D::BeginPass();

			Renderer2D::DrawRenderBuffer();
		
			Renderer2D::EndPass();
		}

		if (m_passSpecification.drawSkybox)
		{
			//g_pEnv->pLevel->GetSkybox()->Draw();
		}

		Renderer3D::EndPass();

		if (m_passSpecification.enableBlending)
		{
			RenderCommand::EnableBlending(false);
		}

		m_passSpecification.targetFramebuffer->Unbind();

		for (const auto& commandSpec : m_passSpecification.framebufferCommands)
		{
			if (!commandSpec.primary || !commandSpec.secondary)
			{
				LP_CORE_ERROR("Framebuffer was nullptr at {0}!", commandSpec.name);
				continue;
			}

			switch (commandSpec.command)
			{
				case FramebufferCommand::Copy:
					commandSpec.primary->Copy(commandSpec.secondary->GetRendererID(), { commandSpec.primary->GetSpecification().Width, commandSpec.primary->GetSpecification().Height }, true);
					break;

				default:
					break;
			}
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

	std::vector<Ref<RenderPass>>& RenderPassManager::GetRenderPasses()
	{
		return m_RenderPasses;
	}

	void RenderPassManager::RenderPasses(Ref<CameraBase>& camera)
	{
		LP_PROFILE_FUNCTION();
		Renderer3D::Begin(camera);
		for (auto& pass : m_RenderPasses)
		{
			pass->Render(camera);
		}
		Renderer3D::End();
	}
}