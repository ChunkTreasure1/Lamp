#include "lppch.h"
#include "PointLightComponent.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Rendering/LightBase.h"

#include "Lamp/Objects/Entity/ComponentRegistry.h"

#include "Lamp/Level/LevelManager.h"
#include "Lamp/Level/Level.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(PointLightComponent);

	PointLightComponent::PointLightComponent()
		: EntityComponent("LightComponent")
	{
		m_pPointLight = CreateScope<PointLight>();
		if (LevelManager::IsLevelLoaded())
		{
			LevelManager::GetActive()->GetEnvironment().RegisterPointLight(m_pPointLight.get());
		}
	}

	PointLightComponent::~PointLightComponent()
	{
		if (!LevelManager::IsLevelLoaded())
		{
			LP_CORE_ERROR("Trying to unregister when no level was loaded!");
		}

		LevelManager::GetActive()->GetEnvironment().UnregisterPointLight(m_pPointLight.get());
	}

	void PointLightComponent::Initialize()
	{
		m_pPointLight->shadowBuffer->SetPosition(m_pEntity->GetPosition());
	}

	void PointLightComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ObjectPositionChangedEvent>(LP_BIND_EVENT_FN(PointLightComponent::OnPositionChanged));
		dispatcher.Dispatch<ObjectPropertyChangedEvent>(LP_BIND_EVENT_FN(PointLightComponent::OnPropertyChanged));
	}

	void PointLightComponent::SetComponentProperties()
	{
		m_componentProperties =
		{
			{ PropertyType::Float, "Intensity", RegisterData(&m_pPointLight->intensity) },
			{ PropertyType::Float, "Radius", RegisterData(&m_pPointLight->radius) },
			{ PropertyType::Float, "Falloff", RegisterData(&m_pPointLight->falloff) },
			{ PropertyType::Float, "Near plane", RegisterData(&m_pPointLight->shadowBuffer->GetNearPlane()) },
			{ PropertyType::Float, "Far plane", RegisterData(&m_pPointLight->farPlane) },
			{ PropertyType::Color3, "Color", RegisterData(&m_pPointLight->color) }
		};
	}

	bool PointLightComponent::OnPositionChanged(ObjectPositionChangedEvent& e)
	{
		m_pPointLight->shadowBuffer->SetPosition(m_pEntity->GetPosition());

		return false;
	}

	bool PointLightComponent::OnPropertyChanged(ObjectPropertyChangedEvent& e)
	{
		m_pPointLight->shadowBuffer->UpdateProjection();

		return false;
	}
}