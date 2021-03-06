#include "lppch.h"
#include "CameraComponent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(CameraComponent);

	void CameraComponent::Initialize()
	{
		SetComponentProperties
		({
			{ PropertyType::Bool, "Is Perspective", RegisterData(&m_IsPerspective) },
			{ PropertyType::Bool, "Is Main", RegisterData(&m_IsMain) },
			{ PropertyType::Float, "Field Of View", RegisterData(&m_PerspectiveCamera->GetFOV()) }
		});
	}

	void CameraComponent::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(CameraComponent::OnUpdate));
	}

	bool CameraComponent::OnUpdate(AppUpdateEvent& e)
	{
		m_PerspectiveCamera->SetPosition(m_pEntity->GetPosition());
		m_PerspectiveCamera->SetRotation(m_pEntity->GetRotation());

		return false;
	}

}