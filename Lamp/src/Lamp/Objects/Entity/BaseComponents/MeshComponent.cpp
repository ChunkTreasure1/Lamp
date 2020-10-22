#include "lppch.h"

#include "MeshComponent.h"

#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(MeshComponent);

	void MeshComponent::Initialize()
	{
	}

	void MeshComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(MeshComponent::OnRender));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(MeshComponent::OnUpdate));
	}

	bool MeshComponent::OnRender(AppRenderEvent& e)
	{
		if (m_Model == nullptr)
		{
			return false;
		}

		m_Model->Render();

		if (g_pEnv->ShouldRenderBB && !e.GetPassInfo().IsShadowPass)
		{
			m_Model->RenderBoundingBox();
		}

		return true;
	}

	bool MeshComponent::OnUpdate(AppUpdateEvent& e)
	{
		if (m_Model)
		{
			m_Model->SetModelMatrix(m_pEntity->GetModelMatrix());
		}

		return true;
	}
}