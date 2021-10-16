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
		m_pDirectionalLight->Transform = m_pEntity->GetTransform();

		glm::vec3 dir = glm::normalize(glm::mat3(m_pDirectionalLight->Transform) * glm::vec3(1.f)) * -1.f;

		glm::mat4 view = glm::lookAt(glm::vec3(0.f) - dir * m_size, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 projection = glm::ortho(-m_size, m_size, -m_size, m_size, 0.1f, 100.f);
		m_pDirectionalLight->ViewProjection = projection * view;
	}
}