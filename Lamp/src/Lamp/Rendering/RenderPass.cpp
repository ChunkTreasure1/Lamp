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

			case PassType::Lightning:
			{
				m_PassSpec.TargetFramebuffer->Bind();
				RenderCommand::Clear();
				Renderer3D::BeginPass(m_PassSpec);

				Renderer3D::CombineLightning();
				
				Renderer3D::EndPass();
				m_PassSpec.TargetFramebuffer->Unbind();

				Renderer3D::CopyDepth();
				break;
			}

			case PassType::SSAO:
			{
				m_PassSpec.TargetFramebuffer->Bind();
				RenderCommand::ClearColor();
				Renderer3D::BeginPass(m_PassSpec);

				Renderer3D::SSAOMainPass();

				Renderer3D::EndPass();
				m_PassSpec.TargetFramebuffer->Unbind();
				break;
			}

			case PassType::SSAOBlur:
			{
				m_PassSpec.TargetFramebuffer->Bind();
				RenderCommand::ClearColor();
				Renderer3D::BeginPass(m_PassSpec);

				Renderer3D::SSAOBlurPass();

				Renderer3D::EndPass();
				m_PassSpec.TargetFramebuffer->Unbind();
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
				RenderCommand::Clear();

				m_PassSpec.TargetFramebuffer->Bind();
				RenderCommand::Clear();

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