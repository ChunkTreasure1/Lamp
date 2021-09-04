#include "lppch.h"

#include "MeshComponent.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/AssetSystem/AssetManager.h"

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
		dispatcher.Dispatch<EntityPropertyChangedEvent>(LP_BIND_EVENT_FN(MeshComponent::OnPropertyChanged));
	}

	bool MeshComponent::OnRender(AppRenderEvent& e)
	{
		if (m_Model == nullptr)
		{
			return false;
		}

		m_Model->Render(m_pEntity->GetID());

		if (g_pEnv->ShouldRenderBB && (e.GetPassInfo().type != PassType::DirectionalShadow && e.GetPassInfo().type != PassType::PointShadow))
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

		return false;
	}

	bool MeshComponent::OnPropertyChanged(EntityPropertyChangedEvent& e)
	{
		if (m_Model == nullptr)
		{
			m_Model = CreateRef<Model>();
		}
		g_pEnv->pAssetManager->LoadModel(m_Path, m_Model.get());

		return false;
	}
}