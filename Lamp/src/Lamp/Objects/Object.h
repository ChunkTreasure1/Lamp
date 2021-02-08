#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "Lamp/Event/Event.h"
#include "Lamp/Event/EntityEvent.h"

#include "Lamp/Math/Math.h"
#include "Lamp/Physics/Internal/Picking.h"

namespace Lamp
{
<<<<<<< HEAD
	class Rigidbody;
=======
	static uint32_t s_ObjectId = 0;
>>>>>>> renderer

	class Object
	{
	public:
<<<<<<< HEAD
		Object();
		~Object();

		//Setting
		void SetPosition(const glm::vec3& pos);
		void SetPhysicsPosition(const glm::vec3& pos);
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; CalculateModelMatrix(); UpdatedMatrix(); }
		inline void AddRotation(const glm::vec3& rot) { m_Rotation += rot; CalculateModelMatrix(); UpdatedMatrix(); }
		void SetScale(const glm::vec3& scale);
=======
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
		inline void SetPhysicsPosition(const glm::vec3& pos) { m_Position = pos; CalculateModelMatrix(); UpdatedMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; CalculateModelMatrix(); UpdatedMatrix(); }
		inline void AddRotation(const glm::vec3& rot) { m_Rotation += rot; CalculateModelMatrix(); UpdatedMatrix(); }
		inline void SetScale(const glm::vec3& scale) { m_Scale = scale; CalculateModelMatrix(); UpdatedMatrix(); }

		void SetModelMatrix(const glm::mat4& mat)
		{
			m_ModelMatrix = mat;

			glm::vec3 scale;
			glm::vec3 rotation;
			glm::vec3 position;

			Math::DecomposeTransform(m_ModelMatrix, position, rotation, scale);

			rotation = rotation - m_Rotation;
>>>>>>> renderer

		void SetModelMatrix(const glm::mat4& mat);
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetLayerID(uint32_t id) { m_LayerID = id; }

		inline void SetIsFrozen(bool state) { m_IsFrozen = state; }
<<<<<<< HEAD
		inline void SetIsActive(bool state) { m_IsActive = state; }
		inline void SetRigidbody(Rigidbody* pBody) { m_pRigidBody = pBody; }
=======
		inline void SetIsActive(bool state) { m_IsActive = state; m_PhysicalEntity->SetIsActive(state); }
		inline void SetIsSelected(bool state) { m_IsSelected = state; }

>>>>>>> renderer
		//Getting
		inline const glm::vec3& GetPosition() { return m_Position; }
		inline const glm::vec3& GetRotation() { return m_Rotation; }
		inline const glm::vec3& GetScale() { return m_Scale; }

		inline const glm::mat4& GetModelMatrix() { return m_ModelMatrix; }
		inline const std::string& GetName() { return m_Name; }
<<<<<<< HEAD
		inline Rigidbody* GetRigidbody() { return m_pRigidBody; }
=======
>>>>>>> renderer

		inline uint32_t GetLayerID() { return m_LayerID; }
		inline bool GetIsFrozen() { return m_IsFrozen; }
		inline bool GetIsActive() { return m_IsActive; }
<<<<<<< HEAD
		inline const PickingCollider& GetPickingCollider() { return m_PickingCollider; }
=======
		
		inline const uint32_t GetID() { return m_Id; }
		inline const bool GetIsSelected() { return m_IsSelected; }
>>>>>>> renderer

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
<<<<<<< HEAD
=======
		bool m_IsSelected = false;
		Ref<PhysicalEntity> m_PhysicalEntity;
>>>>>>> renderer

		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };
		glm::vec3 m_Scale = { 0.f, 0.f, 0.f };

		glm::mat4 m_ModelMatrix = glm::mat4(1.f);
		std::string m_Name;
<<<<<<< HEAD
		uint32_t m_LayerID;

		Rigidbody* m_pRigidBody = nullptr;
		PickingCollider m_PickingCollider;
=======
		uint32_t m_LayerID = 0;
		uint32_t m_Id = 0;
>>>>>>> renderer
	};
}