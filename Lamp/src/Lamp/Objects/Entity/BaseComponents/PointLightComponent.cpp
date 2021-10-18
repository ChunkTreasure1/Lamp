#include "lppch.h"
#include "PointLightComponent.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Rendering/LightBase.h"
#include "Lamp/Level/Level.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(PointLightComponent);

	PointLightComponent::PointLightComponent()
		: EntityComponent("LightComponent")
	{
		{
			FramebufferSpecification spec;
			spec.Height = 512;
			spec.Width = 512;

			m_pPointLight = new PointLight();

			m_pPointLight->shadowBuffer = std::make_shared<PointShadowBuffer>(spec);
		}

		SetComponentProperties
		({
			{ PropertyType::Float, "Intensity", RegisterData(&m_pPointLight->intensity) },
			{ PropertyType::Float, "Radius", RegisterData(&m_pPointLight->radius) },
			{ PropertyType::Float, "Falloff", RegisterData(&m_pPointLight->falloff) },
			{ PropertyType::Float, "Near plane", RegisterData(&m_pPointLight->shadowBuffer->GetNearPlane()) },
			{ PropertyType::Float, "Far plane", RegisterData(&m_pPointLight->farPlane) },
			{ PropertyType::Color3, "Color", RegisterData(&m_pPointLight->color) }
		});

		g_pEnv->pLevel->GetRenderUtils().RegisterPointLight(m_pPointLight);
	}

	PointLightComponent::~PointLightComponent()
	{
		g_pEnv->pLevel->GetRenderUtils().UnregisterPointLight(m_pPointLight);

		delete m_pPointLight;
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