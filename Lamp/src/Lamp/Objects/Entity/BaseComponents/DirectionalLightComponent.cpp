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

		const float size = 20.f;
		glm::vec3 dir = glm::normalize(glm::mat3(m_pDirectionalLight->Transform) * glm::vec3(1.f)) * -1.f;

		glm::mat4 view = glm::lookAt(glm::vec3(0.f) - dir * size, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 projection = glm::ortho(-size, size, -size, size, 0.1f, 100.f);
		m_pDirectionalLight->ViewProjection = projection * view;

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
		m_pDirectionalLight->Transform = m_pEntity->GetModelMatrix();

		const float size = 10.f;
		glm::vec3 dir = glm::normalize(glm::mat3(m_pDirectionalLight->Transform) * glm::vec3(1.f)) * -1.f;

		glm::mat4 view = glm::lookAt(glm::vec3(0.f) - dir * size, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 projection = glm::ortho(-size, size, -size, size, 0.1f, 100.f);
		m_pDirectionalLight->ViewProjection = projection * view;

		return false;
	}
}