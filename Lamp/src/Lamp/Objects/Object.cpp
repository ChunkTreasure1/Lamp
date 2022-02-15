#include "lppch.h"
#include "Object.h"

#include <glm/gtx/quaternion.hpp>

namespace Lamp
{
	static uint32_t s_ObjectId = 0;

	Object::Object()
		: m_position(0.f), m_rotation(0.f), m_scale(1.f), m_transform(1.f), m_name(""), m_layer(0)
	{
		m_id = s_ObjectId++;
	}

	Object::~Object()
	{
	}

	void Object::SetPosition(const glm::vec3 &pos)
	{
		m_position = pos;
		ObjectPositionChangedEvent e;
		OnEvent(e);
	}

	void Object::SetPhysicsPosition(const glm::vec3& pos)
	{
		m_position = pos; 
	}

	void Object::SetRotation(const glm::vec3& rot)
	{
		m_rotation = rot;
		ObjectRotationChangedEvent e;
		OnEvent(e);
	}

	void Object::AddRotation(const glm::vec3& rot)
	{
		m_rotation += rot; 
		ObjectRotationChangedEvent e;
		OnEvent(e);
	}

	void Object::SetScale(const glm::vec3& scale)
	{
		m_scale = scale;
		ObjectScaleChangedEvent e;
		OnEvent(e);
	}

	void Object::SetTransform(const glm::mat4& mat)
	{
		m_transform = mat;

		glm::vec3 scale;
		glm::vec3 rotation;
		glm::vec3 position;

		Math::DecomposeTransform(m_transform, position, rotation, scale);

		rotation = rotation - m_rotation;

		m_rotation += rotation;
		m_position = position;

		m_scale = scale;
	}

	const glm::mat4& Object::GetTransform()
	{
		m_transform = glm::translate(glm::mat4(1.f), m_position)
			* glm::toMat4(glm::quat(m_rotation))
			* glm::scale(glm::mat4(1.f), m_scale);

		return m_transform;
	}
}