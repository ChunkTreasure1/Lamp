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

		FramebufferSpecification spec;
		spec.Height = 4096;
		spec.Width = 4096;
		spec.Attachments =
		{
			{ FramebufferTextureFormat::DEPTH32F, FramebufferTexureFiltering::Nearest, FramebufferTextureWrap::ClampToEdge }
		};

		m_pDirectionalLight->ShadowBuffer = Framebuffer::Create(spec);

		SetComponentProperties
		({
			{ PropertyType::Float, "Intensity", RegisterData(&m_pDirectionalLight->Intensity) },
			{ PropertyType::Color3, "Color", RegisterData(&m_pDirectionalLight->Color) },
			{ PropertyType::Bool, "Cast Shadows", RegisterData(&m_pDirectionalLight->CastShadows) }
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
		glm::vec3 rot = { glm::radians(m_pEntity->GetRotation().x), glm::radians(m_pEntity->GetRotation().y), glm::radians(m_pEntity->GetRotation().z) };
		m_pDirectionalLight->Direction = glm::normalize(glm::vec3(0.f, 0.f, -1.f) * glm::quat(rot));

		return false;
	}
}