#include "lppch.h"
#include "Entity.h"

#include "EntityManager.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"

#include <glm/ext/matrix_transform.hpp>

namespace Lamp
{
void Entity::OnEvent(Event& e)
{
    for (auto it = m_pComponents.begin(); it != m_pComponents.end(); it++)
    {
        if (m_pComponents.size() == 0)
        {
            return;
        }

        if (it->get()->GetEventMask() & e.GetEventType())
        {
            it->get()->OnEvent(e);
        }
    }

    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Entity::OnRenderEvent));
}

void Entity::Destroy()
{
    EntityManager::Get()->Remove(this);
    ObjectLayerManager::Get()->Remove(this);

    delete this;
}

Entity* Entity::Create()
{
    return EntityManager::Get()->Create(false);
}

bool Entity::OnRenderEvent(AppRenderEvent& e)
{
    if (e.GetPassInfo().IsShadowPass || e.GetPassInfo().IsPointShadowPass)
    {
        return false;
    }

    if (g_pEnv->ShouldRenderGizmos)
    {
        if (!m_GizmoShader)
        {
            m_GizmoShader = ShaderLibrary::GetShader("Sprite");
        }

        if (!m_GizmoShader)
        {
            return false;
        }

        m_GizmoShader->Bind();

        m_GizmoShader->UploadMat4("u_ViewProjection", e.GetPassInfo().Camera->GetViewProjectionMatrix());

        glm::vec3 dir = glm::normalize(e.GetPassInfo().Camera->GetPosition() - m_Position);

        float angleXZ = std::atan2f(dir.z, dir.x);
        float angleY = -std::asin(dir.y);

        glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -angleXZ + glm::radians(90.f), { 0.f, 1.f, 0.f })
                             * glm::rotate(glm::mat4(1.f), angleY, { 1.f, 0.f, 0.f });

        glm::mat4 model = glm::translate(glm::mat4(1.f), m_Position)
                          * rotation
                          * glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
        m_GizmoShader->UploadMat4("u_Model", model);
        m_GizmoShader->UploadInt("u_Texture", 0);
        m_GizmoShader->UploadInt("u_ObjectId", m_Id);

        m_GizmoTexure->Bind(0);

        Renderer3D::DrawQuad();
    }

    return false;
}
}