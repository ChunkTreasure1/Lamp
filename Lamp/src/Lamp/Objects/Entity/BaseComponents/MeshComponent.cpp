#include "lppch.h"

#include "MeshComponent.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(MeshComponent);

	MeshComponent::~MeshComponent()
	{
		g_pEnv->pLevel->GetRenderUtils().UnegisterMeshComponent(m_pEntity->GetID());
	}

	void MeshComponent::Initialize()
	{
		g_pEnv->pLevel->GetRenderUtils().RegisterMeshComponent(m_pEntity->GetID(), this);
	}

	void MeshComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(MeshComponent::OnRender));
		dispatcher.Dispatch<EntityPropertyChangedEvent>(LP_BIND_EVENT_FN(MeshComponent::OnPropertyChanged));
	}

	bool MeshComponent::OnRender(AppRenderEvent& e)
	{
		if (m_Model == nullptr)
		{
			return false;
		}

		m_Model->Render(m_pEntity->GetID(), m_pEntity->GetModelMatrix());

		return true;
	}

	bool MeshComponent::OnPropertyChanged(EntityPropertyChangedEvent& e)
	{
		m_Model = ResourceCache::GetAsset<Mesh>(m_Path);

		return false;
	}
}