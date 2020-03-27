#pragma once
#include <glm/glm.hpp>

namespace Lamp
{
	class PhysicsObject
	{
	public:
		PhysicsObject()
			: m_Position(0, 0, 0)
		{}

		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; }
		inline const glm::vec3& GetPosition() { return m_Position; }

	protected:
		glm::vec3 m_Position;
	};
}