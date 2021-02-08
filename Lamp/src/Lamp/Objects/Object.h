#pragma once

#include "Entity/Base/Physical/PhysicalEntity.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Lamp/Event/Event.h"
#include "Lamp/Event/EntityEvent.h"

#include "Lamp/Math/Math.h"

namespace Lamp
{
static uint32_t s_ObjectId = 0;

class Object
{
public:
    Object()
        : m_Position(0.f), m_Rotation(0.f), m_Scale(1.f), m_ModelMatrix(1.f), m_Name(""), m_LayerID(0)
    {
        m_Id = s_ObjectId++;
    }

    //Setting
    void SetPosition(const glm::vec3& pos)
    {
        m_Position = pos;
        m_PhysicalEntity->GetCollider()->SetTranslation(pos);
        CalculateModelMatrix();
        UpdatedMatrix();

        EntityPositionChangedEvent e;
        OnEvent(e);
    }
    inline void SetPhysicsPosition(const glm::vec3& pos) {
        m_Position = pos;
        CalculateModelMatrix();
        UpdatedMatrix();
    }
    inline void SetRotation(const glm::vec3& rot) {
        m_Rotation = rot;
        CalculateModelMatrix();
        UpdatedMatrix();
    }
    inline void AddRotation(const glm::vec3& rot) {
        m_Rotation += rot;
        CalculateModelMatrix();
        UpdatedMatrix();
    }
    inline void SetScale(const glm::vec3& scale) {
        m_Scale = scale;
        CalculateModelMatrix();
        UpdatedMatrix();
    }

    void SetModelMatrix(const glm::mat4& mat)
    {
        m_ModelMatrix = mat;

        glm::vec3 scale;
        glm::vec3 rotation;
        glm::vec3 position;

        Math::DecomposeTransform(m_ModelMatrix, position, rotation, scale);

        rotation = rotation - m_Rotation;

        m_Rotation += rotation;
        m_Position = position;
        m_PhysicalEntity->GetCollider()->SetTranslation(position);

        m_Scale = scale;
    }
    inline void SetName(const std::string& name) {
        m_Name = name;
    }
    inline void SetLayerID(uint32_t id) {
        m_LayerID = id;
    }

    inline void SetIsFrozen(bool state) {
        m_IsFrozen = state;
    }
    inline void SetIsActive(bool state) {
        m_IsActive = state;
        m_PhysicalEntity->SetIsActive(state);
    }
    inline void SetIsSelected(bool state) {
        m_IsSelected = state;
    }

    //Getting
    inline const glm::vec3& GetPosition() {
        return m_Position;
    }
    inline const glm::vec3& GetRotation() {
        return m_Rotation;
    }
    inline const glm::vec3& GetScale() {
        return m_Scale;
    }

    inline const glm::mat4& GetModelMatrix() {
        return m_ModelMatrix;
    }
    inline Ref<PhysicalEntity>& GetPhysicalEntity() {
        return m_PhysicalEntity;
    }
    inline const std::string& GetName() {
        return m_Name;
    }

    inline uint32_t GetLayerID() {
        return m_LayerID;
    }
    inline bool GetIsFrozen() {
        return m_IsFrozen;
    }
    inline bool GetIsActive() {
        return m_IsActive;
    }

    inline const uint32_t GetID() {
        return m_Id;
    }
    inline const bool GetIsSelected() {
        return m_IsSelected;
    }

    virtual void OnEvent(Event& e) {}
    virtual uint64_t GetEventMask() = 0;
    virtual void Destroy() = 0;

protected:
    void CalculateModelMatrix()
    {
        m_ModelMatrix = glm::translate(glm::mat4(1.f), m_Position)
                        * glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.x), glm::vec3(1.f, 0.f, 0.f))
                        * glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.y), glm::vec3(0.f, 1.f, 0.f))
                        * glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.z), glm::vec3(0.f, 0.f, 1.f))
                        * glm::scale(glm::mat4(1.f), m_Scale);
    }

    virtual void UpdatedMatrix() {}

protected:
    bool m_IsActive = true;
    bool m_IsFrozen = false;
    bool m_IsSelected = false;
    Ref<PhysicalEntity> m_PhysicalEntity;

    glm::vec3 m_Position = { 0.f, 0.f, 0.f };
    glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };
    glm::vec3 m_Scale = { 0.f, 0.f, 0.f };

    glm::mat4 m_ModelMatrix = glm::mat4(1.f);
    std::string m_Name;
    uint32_t m_LayerID = 0;
    uint32_t m_Id = 0;
};
}