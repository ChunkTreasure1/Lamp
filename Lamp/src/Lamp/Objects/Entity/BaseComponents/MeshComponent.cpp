#include "lppch.h"

#include "MeshComponent.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/AssetSystem/ResourceCache.h"

#include "Lamp/Level/Level.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(MeshComponent);

	MeshComponent::MeshComponent()
		: EntityComponent("MeshComponent")
	{}

	MeshComponent::~MeshComponent()
	{}

	void MeshComponent::Initialize()
	{
	}

	void MeshComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(MeshComponent::OnRender));
		dispatcher.Dispatch<ObjectPropertyChangedEvent>(LP_BIND_EVENT_FN(MeshComponent::OnPropertyChanged));
	}

	void MeshComponent::SetComponentProperties()
	{
		m_componentProperties =
		{
			{ PropertyType::Path, "Path", RegisterData(&m_path) }
		};
	}

	bool MeshComponent::OnRender(AppRenderEvent& e)
	{
		if (m_mesh == nullptr)
		{
			return false;
		}

		m_mesh->Render(m_pEntity->GetID(), m_pEntity->GetTransform());

		return true;
	}

	bool MeshComponent::OnPropertyChanged(ObjectPropertyChangedEvent& e)
	{
		m_mesh = ResourceCache::GetAsset<Mesh>(m_path);

		return false;
	}
}