#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "Lamp/Event/Event.h"
#include "Lamp/Event/EntityEvent.h"

#include "Lamp/Math/Math.h"
#include "Lamp/Physics/Internal/Picking.h"

namespace Lamp
{
	class Rigidbody;

	class Object
	{
	public:
		Object();
		~Object();

		//Setting
		void SetPosition(const glm::vec3& pos);
		void SetPhysicsPosition(const glm::vec3& pos);
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; CalculateModelMatrix(); UpdatedMatrix(); }
		inline void AddRotation(const glm::vec3& rot) { m_Rotation += rot; CalculateModelMatrix(); UpdatedMatrix(); }
		void SetScale(const glm::vec3& scale);

		void SetModelMatrix(const glm::mat4& mat);
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetLayerID(uint32_t id) { m_LayerID = id; }
		
		inline void SetIsFrozen(bool state) { m_IsFrozen = state; }
		inline void SetIsActive(bool state) { m_IsActive = state; }
		inline void SetRigidbody(Rigidbody* pBody) { m_pRigidBody = pBody; }
		//Getting
		inline const glm::vec3& GetPosition() { return m_Position; }
		inline const glm::vec3& GetRotation() { return m_Rotation; }
		inline const glm::vec3& GetScale() { return m_Scale; }

		inline const glm::mat4& GetModelMatrix() { return m_ModelMatrix; }
		inline const std::string& GetName() { return m_Name; }
		inline Rigidbody* GetRigidbody() { return m_pRigidBody; }

		inline uint32_t GetLayerID() { return m_LayerID; }
		inline bool GetIsFrozen() { return m_IsFrozen; }
		inline bool GetIsActive() { return m_IsActive; }
		inline const PickingCollider& GetPickingCollider() { return m_PickingCollider; }

		virtual void OnEvent(Event& e) {}
		virtual uint64_t GetEventMask() = 0;
		virtual void Destroy() = 0;

	protected:
		void CalculateModelMatrix();
		virtual void UpdatedMatrix() {}
		virtual void ScaleChanged() {}

	protected:
		bool m_IsActive = true;
		bool m_IsFrozen = false;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		glm::mat4 m_ModelMatrix;
		std::string m_Name;
		uint32_t m_LayerID;

		Rigidbody* m_pRigidBody = nullptr;
		PickingCollider m_PickingCollider;
	};
}