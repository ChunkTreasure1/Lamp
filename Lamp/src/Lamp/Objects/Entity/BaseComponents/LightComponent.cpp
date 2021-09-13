#include "lppch.h"
#include "LightComponent.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Level/Level.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(LightComponent);

	LightComponent::LightComponent()
		: EntityComponent("LightComponent")
	{
		{
			FramebufferSpecification spec;
			spec.Height = 512;
			spec.Width = 512;

			m_pPointLight = new PointLight();

			m_pPointLight->ShadowBuffer = std::make_shared<PointShadowBuffer>(spec);
			g_pEnv->pLevel->GetRenderUtils().RegisterPointLight(m_pPointLight);
		}


		SetComponentProperties
		({
			{ PropertyType::Float, "Intensity", RegisterData(&m_pPointLight->Intensity) },
			{ PropertyType::Float, "Radius", RegisterData(&m_pPointLight->Radius) },
			{ PropertyType::Float, "Falloff", RegisterData(&m_pPointLight->Falloff) },
			{ PropertyType::Float, "Near plane", RegisterData(&m_pPointLight->ShadowBuffer->GetNearPlane()) },
			{ PropertyType::Color3, "Color", RegisterData(&m_pPointLight->Color) }
		});
	}

	LightComponent::~LightComponent()
	{
		g_pEnv->pLevel->GetRenderUtils().UnregisterPointLight(m_pPointLight);

		delete m_pPointLight;
	}

	void LightComponent::Initialize()
	{
		m_pPointLight->ShadowBuffer->SetPosition(m_pEntity->GetPosition());
	}

	void LightComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EntityPositionChangedEvent>(LP_BIND_EVENT_FN(LightComponent::OnPositionChanged));
		dispatcher.Dispatch<EntityPropertyChangedEvent>(LP_BIND_EVENT_FN(LightComponent::OnPropertyChanged));
	}

	bool LightComponent::OnPositionChanged(EntityPositionChangedEvent& e)
	{
		m_pPointLight->ShadowBuffer->SetPosition(m_pEntity->GetPosition());

		return false;
	}

	bool LightComponent::OnPropertyChanged(EntityPropertyChangedEvent& e)
	{
		m_pPointLight->ShadowBuffer->UpdateProjection();

		return false;
	}
}