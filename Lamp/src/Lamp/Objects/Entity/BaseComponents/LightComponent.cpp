#include "lppch.h"
#include "LightComponent.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"

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

			m_PointLight.ShadowBuffer = std::make_shared<PointShadowBuffer>(spec);
			g_pEnv->pRenderUtils->RegisterPointLight(m_PointLight);
		}


		SetComponentProperties
		({
			{ PropertyType::Float, "Intensity", RegisterData(&m_PointLight.Intensity) },
			{ PropertyType::Float, "Radius", RegisterData(&m_PointLight.Radius) },
			{ PropertyType::Float, "Falloff", RegisterData(&m_PointLight.Falloff) },
			{ PropertyType::Color3, "Color", RegisterData(&m_PointLight.Color) }
		});
	}

	LightComponent::~LightComponent()
	{
		g_pEnv->pRenderUtils->UnregisterPointLight(m_PointLight);
	}

	void LightComponent::Initialize()
	{
		m_PointLight.ShadowBuffer->SetPosition(m_pEntity->GetPosition());
	}

	void LightComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EntityPositionChangedEvent>(LP_BIND_EVENT_FN(LightComponent::OnPositionChanged));
	}

	bool LightComponent::OnRender(AppRenderEvent& e)
	{
		return false;
	}

	bool LightComponent::OnUpdate(AppUpdateEvent& e)
	{
		return false;
	}

	bool LightComponent::OnPositionChanged(EntityPositionChangedEvent& e)
	{
		m_PointLight.ShadowBuffer->SetPosition(m_pEntity->GetPosition());

		return false;
	}
}