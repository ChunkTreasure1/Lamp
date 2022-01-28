#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "Lamp/Event/Event.h"
#include "Lamp/Event/EntityEvent.h"

#include "Lamp/Math/Math.h"
#include "Lamp/Core/UUID.h"

namespace Lamp
{
	class Object
	{
	public:
		Object();
		virtual ~Object() = 0;

		//Setting
		void SetPosition(const glm::vec3& pos);
		void SetPhysicsPosition(const glm::vec3& pos);
		void SetRotation(const glm::vec3& rot);
		void AddRotation(const glm::vec3& rot);
		void SetScale(const glm::vec3& scale);

		void SetModelMatrix(const glm::mat4& mat);
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetLayerID(uint32_t id) { m_LayerID = id; }

		inline void SetIsFrozen(bool state) { m_IsFrozen = state; }
		inline void SetIsActive(bool state) { m_IsActive = state; }
		inline void SetIsSelected(bool state) { m_IsSelected = state; }

		//Getting
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }
		inline const glm::vec3& GetScale() const { return m_Scale; }

		const glm::mat4& GetTransform();
		inline const std::string& GetName() const { return m_Name; }

		inline uint32_t GetLayerID() const { return m_LayerID; }
		inline bool GetIsFrozen() const { return m_IsFrozen; }
		inline bool GetIsActive() const { return m_IsActive; }
		
		inline const uint32_t GetID() const { return m_Id; }
		inline const bool GetIsSelected() const { return m_IsSelected; }

		virtual void OnEvent(Event& e) {}
		virtual void Destroy() = 0;

	protected:
		bool m_IsActive = true;
		bool m_IsFrozen = false;
		bool m_IsSelected = false;

		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };
		glm::vec3 m_Scale = { 0.f, 0.f, 0.f };

		glm::mat4 m_Transform = glm::mat4(1.f);
		std::string m_Name;

		uint32_t m_LayerID = 0;
		uint32_t m_Id;
	};
}