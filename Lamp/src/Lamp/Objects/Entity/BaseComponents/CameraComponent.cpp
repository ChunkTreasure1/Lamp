#include "lppch.h"
#include "CameraComponent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(CameraComponent);

	CameraComponent::CameraComponent()
		: EntityComponent("CameraComponent"), m_farPlane(1000.f), m_nearPlane(0.01f)
	{
		m_perspectiveCamera = CreateRef<PerspectiveCamera>(60.f, m_nearPlane, m_farPlane);
	}

	CameraComponent::~CameraComponent()
	{
	}

	void CameraComponent::Initialize()
	{
		SetComponentProperties
		({
			{ PropertyType::Bool, "Is Perspective", RegisterData(&m_isPerspective) },
			{ PropertyType::Bool, "Is Main", RegisterData(&m_isMain) },
			{ PropertyType::Float, "Field Of View", RegisterData(&m_perspectiveCamera->GetFieldOfView()) },
			{ PropertyType::Float, "Near Plane", RegisterData(&m_nearPlane) },
			{ PropertyType::Float, "Far Plane", RegisterData(&m_farPlane) }
		});
	}

	void CameraComponent::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(CameraComponent::OnUpdate));
		dispatcher.Dispatch<EditorViewportSizeChangedEvent>(LP_BIND_EVENT_FN(CameraComponent::OnViewportSizeChanged));
		dispatcher.Dispatch<ObjectPropertyChangedEvent>(LP_BIND_EVENT_FN(CameraComponent::OnPropertyUpdated));
	}

	bool CameraComponent::OnUpdate(AppUpdateEvent& e)
	{
		m_perspectiveCamera->SetPosition(m_pEntity->GetPosition());
		m_perspectiveCamera->SetRotation(m_pEntity->GetRotation());

		return false;
	}

	bool CameraComponent::OnViewportSizeChanged(EditorViewportSizeChangedEvent& e)
	{
		m_currentAspectRatio = (float)e.GetWidth() / (float)e.GetHeight();

		m_perspectiveCamera->SetProjection(m_perspectiveCamera->GetFieldOfView(), m_currentAspectRatio, m_nearPlane, m_farPlane);
		return false;
	}

	bool CameraComponent::OnPropertyUpdated(ObjectPropertyChangedEvent& e)
	{
		m_perspectiveCamera->SetProjection(m_perspectiveCamera->GetFieldOfView(), m_currentAspectRatio, m_nearPlane, m_farPlane);

		return false;
	}

}