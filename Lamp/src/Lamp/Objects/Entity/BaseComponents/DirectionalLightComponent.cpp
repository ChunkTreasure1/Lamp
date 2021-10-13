#include "lppch.h"
#include "DirectionalLightComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Lamp/Level/Level.h"

#include <glm/gtx/quaternion.hpp>

namespace Lamp
{
	LP_REGISTER_COMPONENT(DirectionalLightComponent)

	DirectionalLightComponent::DirectionalLightComponent()
		: EntityComponent("DirectionalLightComponent")
	{
		m_pDirectionalLight = new DirectionalLight();

		SetComponentProperties
		({
			{ PropertyType::Float, "Intensity", RegisterData(&m_pDirectionalLight->Intensity) },
			{ PropertyType::Color3, "Color", RegisterData(&m_pDirectionalLight->Color) }
		});

		g_pEnv->pLevel->GetRenderUtils().RegisterDirectionalLight(m_pDirectionalLight);
	}

	DirectionalLightComponent::~DirectionalLightComponent()
	{
		g_pEnv->pLevel->GetRenderUtils().UnregisterDirectionalLight(m_pDirectionalLight);
		delete m_pDirectionalLight;
	}

	void DirectionalLightComponent::Initialize()
	{

	}

	void DirectionalLightComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EntityRotationChangedEvent>(LP_BIND_EVENT_FN(DirectionalLightComponent::OnRotationChanged));
	}

	bool DirectionalLightComponent::OnRotationChanged(EntityRotationChangedEvent& e)
	{
		m_pDirectionalLight->Direction = glm::normalize(glm::quat(m_pEntity->GetRotation()) * glm::vec3(0.f, 0.f, -1.f));

		return false;
	}
}