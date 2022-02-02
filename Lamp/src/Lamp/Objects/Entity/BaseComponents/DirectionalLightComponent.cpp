#include "lppch.h"
#include "DirectionalLightComponent.h"

#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Level/LevelManager.h"
#include "Lamp/Level/Level.h"

#include <glm/gtx/quaternion.hpp>

namespace Lamp
{
	LP_REGISTER_COMPONENT(DirectionalLightComponent)

	DirectionalLightComponent::DirectionalLightComponent()
		: EntityComponent("DirectionalLightComponent")
	{
		m_pDirectionalLight = CreateScope<DirectionalLight>();

		SetComponentProperties
		({
			{ PropertyType::Float, "Intensity", RegisterData(&m_pDirectionalLight->intensity) },
			{ PropertyType::Color3, "Color", RegisterData(&m_pDirectionalLight->color) },
			{ PropertyType::Bool, "Cast Shadows", RegisterData(&m_pDirectionalLight->castShadows) }
		});

		if (LevelManager::GetActive())
		{
			auto& env = LevelManager::GetActive()->GetEnvironment();
			env.RegisterDirectionalLight(m_pDirectionalLight.get());
		}
	}

	DirectionalLightComponent::~DirectionalLightComponent()
	{
		if (!LevelManager::GetActive())
		{
			LP_CORE_ERROR("Trying to unregister when no level is loaded!");
			return;
		}

		LevelManager::GetActive()->GetEnvironment().UnregisterDirectionalLight(m_pDirectionalLight.get());
	}

	void DirectionalLightComponent::Initialize()
	{
		UpdateLight();
	}

	void DirectionalLightComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ObjectRotationChangedEvent>(LP_BIND_EVENT_FN(DirectionalLightComponent::OnRotationChanged));
	}

	bool DirectionalLightComponent::OnRotationChanged(ObjectRotationChangedEvent& e)
	{
		UpdateLight();
		return false;
	}

	void DirectionalLightComponent::UpdateLight()
	{
		m_pDirectionalLight->transform = m_pEntity->GetTransform();

		glm::vec3 dir = glm::normalize(glm::mat3(m_pDirectionalLight->transform) * glm::vec3(1.f)) * -1.f;

		glm::mat4 view = glm::lookAt(glm::vec3(0.f) - dir * m_size, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 projection = glm::ortho(-m_size, m_size, -m_size, m_size, 0.1f, 100.f);
		m_pDirectionalLight->viewProjection = projection * view;
	}
}