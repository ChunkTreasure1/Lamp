#pragma once

#include "Entity/Base/Physical/PhysicalEntity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Lamp/Event/Event.h"
#include "Lamp/Event/EntityEvent.h"

namespace Lamp
{
	class Object
	{
	public:
		Object()
			: m_Position(0.f), m_Rotation(0.f), m_Scale(1.f), m_ModelMatrix(1.f), m_Name(""), m_LayerID(0)
		{}

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
		inline void SetPhysicsPosition(const glm::vec3& pos) { m_Position = pos; CalculateModelMatrix(); UpdatedMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; CalculateModelMatrix(); UpdatedMatrix(); }
		inline void SetScale(const glm::vec3& scale) { m_Scale = scale; CalculateModelMatrix(); UpdatedMatrix(); }

		void SetModelMatrix(const glm::mat4& mat) 
		{ 
			m_ModelMatrix = mat;

			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 position;
			glm::vec3 skew;
			glm::vec4 perspective;

			glm::decompose(m_ModelMatrix, scale, rotation, position, skew, perspective);
			rotation = glm::conjugate(rotation);
			
			m_Rotation = glm::eulerAngles(rotation);
			m_Rotation.x = -glm::degrees(m_Rotation.x);
			m_Rotation.y = -glm::degrees(m_Rotation.y);
			m_Rotation.z = -glm::degrees(m_Rotation.z);

			m_Position = position;
			m_PhysicalEntity->GetCollider()->SetTranslation(position);

			m_Scale = scale;
		}
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetLayerID(uint32_t id) { m_LayerID = id; }
		
		inline void SetIsFrozen(bool state) { m_IsFrozen = state; }
		inline void SetIsActive(bool state) { m_IsActive = state; m_PhysicalEntity->SetIsActive(state); }

		//Getting
		inline const glm::vec3& GetPosition() { return m_Position; }
		inline const glm::vec3& GetRotation() { return m_Rotation; }
		inline const glm::vec3& GetScale() { return m_Scale; }

		inline glm::mat4& GetModelMatrix() { return m_ModelMatrix; }
		inline Ref<PhysicalEntity>& GetPhysicalEntity() { return m_PhysicalEntity; }
		inline const std::string& GetName() { return m_Name; }
		
		inline uint32_t GetLayerID() { return m_LayerID; }
		inline bool GetIsFrozen() { return m_IsFrozen; }
		inline bool GetIsActive() { return m_IsActive; }

		virtual void OnEvent(Event& e) {}
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
		Ref<PhysicalEntity> m_PhysicalEntity;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		glm::mat4 m_ModelMatrix;
		std::string m_Name;
		uint32_t m_LayerID;
	};
}