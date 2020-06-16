#include "lppch.h"
#include "LightComponent.h"

namespace Lamp
{
	bool LightComponent::s_Registered = LP_REGISTER_COMPONENT(LightComponent);

	void LightComponent::Initialize()
	{
	}

	void LightComponent::OnEvent(Event& e)
	{
	}

	bool LightComponent::OnRender(AppRenderEvent& e)
	{
		return false;
	}

	bool LightComponent::OnUpdate(AppUpdateEvent& e)
	{
		return false;
	}
}