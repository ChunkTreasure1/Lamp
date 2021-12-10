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
	}
}