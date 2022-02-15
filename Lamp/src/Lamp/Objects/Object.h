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

		void SetTransform(const glm::mat4& mat);
		inline void SetName(const std::string& name) { m_name = name; }
		inline void SetLayerID(uint32_t id) { m_layer = id; }

		inline void SetIsFrozen(bool state) { m_isFrozen = state; }
		inline void SetIsActive(bool state) { m_isActive = state; }
		inline void SetIsSelected(bool state) { m_isSelected = state; }

		//Getting
		inline const glm::vec3& GetPosition() const { return m_position; }
		inline const glm::vec3& GetRotation() const { return m_rotation; }
		inline const glm::vec3& GetScale() const { return m_scale; }

		const glm::mat4& GetTransform();
		inline const std::string& GetName() const { return m_name; }

		inline uint32_t GetLayerID() const { return m_layer; }
		inline bool GetIsFrozen() const { return m_isFrozen; }
		inline bool GetIsActive() const { return m_isActive; }
		
		inline const uint32_t GetID() const { return m_id; }
		inline const bool GetIsSelected() const { return m_isSelected; }

		virtual void OnEvent(Event& e) {}
		virtual void Destroy() = 0;

	protected:
		bool m_isActive = true;
		bool m_isFrozen = false;
		bool m_isSelected = false;

		glm::vec3 m_position = { 0.f, 0.f, 0.f };
		glm::vec3 m_rotation = { 0.f, 0.f, 0.f };
		glm::vec3 m_scale = { 0.f, 0.f, 0.f };

		glm::mat4 m_transform = glm::mat4(1.f);
		std::string m_name;

		uint32_t m_layer = 0;
		uint32_t m_id;
	};
}