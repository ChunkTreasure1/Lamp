#include "TransformComponent.h"

namespace LampEntity
{
	void TransformComponent::Initialize()
	{
		m_Position = glm::vec2(0, 0);
		m_Rotation = glm::vec2(0, 0);
		m_Scale = 1.f;
	}
}