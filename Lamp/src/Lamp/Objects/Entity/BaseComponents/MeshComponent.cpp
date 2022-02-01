#include "lppch.h"

#include "MeshComponent.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/AssetSystem/ResourceCache.h"

#include "Lamp/Level/Level.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(MeshComponent);

	MeshComponent::~MeshComponent()
	{
	}

	void MeshComponent::Initialize()
	{
	}

	void MeshComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(MeshComponent::OnRender));
		dispatcher.Dispatch<ObjectPropertyChangedEvent>(LP_BIND_EVENT_FN(MeshComponent::OnPropertyChanged));
	}

	bool MeshComponent::OnRender(AppRenderEvent& e)
	{
		if (m_Model == nullptr)
		{
			return false;
		}

		m_Model->Render(m_pEntity->GetID(), m_pEntity->GetTransform());

		return true;
	}

	bool MeshComponent::OnPropertyChanged(ObjectPropertyChangedEvent& e)
	{
		m_Model = ResourceCache::GetAsset<Mesh>(m_Path);

		return false;
	}
}