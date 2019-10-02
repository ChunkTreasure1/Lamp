#pragma once

#include <glm/glm.hpp>

#include "Lamp/Entity/Base/Component.h"

namespace Lamp
{
	class TransformComponent : public IEntityComponent
	{
	public:
		TransformComponent(const glm::vec2& pos)
			: m_Scale(1.f)
		{
			m_Position = pos;
		}

		virtual void Initialize() override {}
		virtual void Update() override {}
		virtual void Draw() override {}

		//Getting
		inline const glm::vec2& GetPosition() const { return m_Position; }
		inline const glm::vec2& GetRotation() const { return m_Rotation; }
		inline const float GetScale() const { return m_Scale; }

		//Setting
		inline void SetPosition(glm::vec2& pos) { m_Position = pos; }
		inline void SetRotation(glm::vec2& rot) { m_Rotation = rot; }
		inline void SetScale(float s) { m_Scale = s; }

	private:
		glm::vec2 m_Position;
		glm::vec2 m_Rotation;
		float m_Scale;
	};
}