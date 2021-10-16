#include "lppch.h"
#include "Object.h"

#include <glm/gtx/quaternion.hpp>

namespace Lamp
{
	static uint32_t s_ObjectId = 0;

	Object::Object()
		: m_Position(0.f), m_Rotation(0.f), m_Scale(1.f), m_Transform(1.f), m_Name(""), m_LayerID(0)
	{
		m_Id = s_ObjectId++;
	}

	Object::~Object()
	{
	}

	void Object::SetPosition(const glm::vec3 &pos)
	{
		m_Position = pos;
		ObjectPositionChangedEvent e;
		OnEvent(e);
	}

	void Object::SetPhysicsPosition(const glm::vec3& pos)
	{
		m_Position = pos; 
	}

	void Object::SetRotation(const glm::vec3& rot)
	{
		m_Rotation = rot;
		ObjectRotationChangedEvent e;
		OnEvent(e);
	}

	void Object::AddRotation(const glm::vec3& rot)
	{
		m_Rotation += rot; 
		ObjectRotationChangedEvent e;
		OnEvent(e);
	}

	void Object::SetScale(const glm::vec3& scale)
	{
		m_Scale = scale;
		ObjectScaleChangedEvent e;
		OnEvent(e);
	}

	void Object::SetModelMatrix(const glm::mat4& mat)
	{
		m_Transform = mat;

		glm::vec3 scale;
		glm::vec3 rotation;
		glm::vec3 position;

		Math::DecomposeTransform(m_Transform, position, rotation, scale);

		rotation = rotation - m_Rotation;

		m_Rotation += rotation;
		m_Position = position;

		m_Scale = scale;
	}

	const glm::mat4& Object::GetTransform()
	{
		m_Transform = glm::translate(glm::mat4(1.f), m_Position)
			* glm::toMat4(glm::quat(m_Rotation))
			* glm::scale(glm::mat4(1.f), m_Scale);

		return m_Transform;
	}
}