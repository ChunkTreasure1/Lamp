#include "lppch.h"
#include "RenderPass.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Core/Application.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Level/Level.h"

namespace Lamp
{
	Ref<RenderPassManager> RenderPassManager::s_Instance = nullptr;

	RenderPass::RenderPass(const RenderPassSpecification& spec)
		: m_PassSpec(spec)
	{
	}

	void RenderPass::Render(Ref<CameraBase>& camera)
	{
		LP_PROFILE_SCOPE("RenderPass::Render::" + m_PassSpec.Name);

		switch (m_PassSpec.type)
		{
			case PassType::PointShadow:
			{
				m_PassSpec.LightIndex = 0;
				for (auto& light : g_pEnv->pLevel->GetRenderUtils().GetPointLights())
				{
					light->ShadowBuffer->Bind();
					RenderCommand::Clear();

					Renderer3D::BeginPass(m_PassSpec);

					Renderer3D::DrawRenderBuffer();

					Renderer3D::EndPass();
					light->ShadowBuffer->Unbind();
					m_PassSpec.LightIndex++;
				}
				break;
			}

			case PassType::Forward:
			{
				m_PassSpec.TargetFramebuffer->Bind();
				Renderer3D::BeginPass(m_PassSpec);

				Renderer3D::DrawRenderBuffer();

				for (auto& f : m_PassSpec.ExtraRenders)
				{
					f();
				}

				Renderer3D::EndPass();
				m_PassSpec.TargetFramebuffer->Unbind();
				break;
			}

			default:
			{
				RenderCommand::SetClearColor(m_PassSpec.TargetFramebuffer->GetSpecification().ClearColor);
				m_PassSpec.TargetFramebuffer->Bind();

				//Clear color if i should
				switch (m_PassSpec.clearType)
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

				Renderer3D::BeginPass(m_PassSpec);

				switch (m_PassSpec.drawType)
				{
					case DrawType::All:
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

				Renderer3D::EndPass();
				m_PassSpec.TargetFramebuffer->Unbind();

				for (auto& [main, secondary, command] : m_PassSpec.framebufferCommands)
				{
					switch (command)
					{
						case FramebufferCommand::Copy:
							main->Copy(secondary->GetRendererID(), { main->GetSpecification().Width, main->GetSpecification().Height }, true);
							break;

						default:
							break;
					}
				}

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