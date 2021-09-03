#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "Lamp/Event/Event.h"
#include "Lamp/Event/EntityEvent.h"

#include "Lamp/Math/Math.h"

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
		inline void SetIsSelected(bool state) { m_IsSelected = state; }

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
		
		inline const uint32_t GetID() { return m_Id; }
		inline const bool GetIsSelected() { return m_IsSelected; }

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
		bool m_IsSelected = false;

		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };
		glm::vec3 m_Scale = { 0.f, 0.f, 0.f };

		glm::mat4 m_ModelMatrix = glm::mat4(1.f);
		std::string m_Name;

		Rigidbody* m_pRigidBody = nullptr;
		uint32_t m_LayerID = 0;
		uint32_t m_Id = 0;
	};
}