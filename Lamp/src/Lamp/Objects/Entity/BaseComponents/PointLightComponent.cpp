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

			m_pPointLight->ShadowBuffer = std::make_shared<PointShadowBuffer>(spec);
		}

		SetComponentProperties
		({
			{ PropertyType::Float, "Intensity", RegisterData(&m_pPointLight->Intensity) },
			{ PropertyType::Float, "Radius", RegisterData(&m_pPointLight->Radius) },
			{ PropertyType::Float, "Falloff", RegisterData(&m_pPointLight->Falloff) },
			{ PropertyType::Float, "Near plane", RegisterData(&m_pPointLight->ShadowBuffer->GetNearPlane()) },
			{ PropertyType::Float, "Far plane", RegisterData(&m_pPointLight->FarPlane) },
			{ PropertyType::Color3, "Color", RegisterData(&m_pPointLight->Color) }
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
		m_pPointLight->ShadowBuffer->SetPosition(m_pEntity->GetPosition());
	}

	void PointLightComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ObjectPositionChangedEvent>(LP_BIND_EVENT_FN(PointLightComponent::OnPositionChanged));
		dispatcher.Dispatch<ObjectPropertyChangedEvent>(LP_BIND_EVENT_FN(PointLightComponent::OnPropertyChanged));
	}

	bool PointLightComponent::OnPositionChanged(ObjectPositionChangedEvent& e)
	{
		m_pPointLight->ShadowBuffer->SetPosition(m_pEntity->GetPosition());

		return false;
	}

	bool PointLightComponent::OnPropertyChanged(ObjectPropertyChangedEvent& e)
	{
		m_pPointLight->ShadowBuffer->UpdateProjection();

		return false;
	}
}