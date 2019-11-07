#pragma once

#include <glm/glm.hpp>

#include "Lamp/Entity/Base/Component.h"

namespace Lamp
{
	class TransformComponent : public IEntityComponent
	{
	public:
		TransformComponent(const glm::vec3& pos)
			: IEntityComponent("Transform Component"), m_Scale(glm::vec3(1.f, 1.f, 1.f)), m_Rotation(glm::vec3(0, 0, 0))
		{
			m_Position = pos;
			SetComponentProperties
			({
				{ PropertyType::Float3, "Position", static_cast<void*>(&m_Position) },
				{ PropertyType::Float3, "Rotation", static_cast<void*>(&m_Rotation) },
				{ PropertyType::Float3, "Scale", static_cast<void*>(&m_Scale) }
			});
		}

		virtual void Initialize() override {}
		virtual void Update() override {}
		virtual void Draw() override {}

		virtual void SetProperty(ComponentProperty& prop, void* pData) override
		{
			if (prop.Name == "Position")
			{
				glm::vec3* p = std::any_cast<glm::vec3*>(ComponentProperties::GetValue(prop, pData));
				SetPosition({ p->x, p->y, p->z });
			}
			else if (prop.Name == "Rotation")
			{
				glm::vec3* p = std::any_cast<glm::vec3*>(ComponentProperties::GetValue(prop, pData));
				SetRotation({ p->x, p->y, p->z });
			}
			else if (prop.Name == "Scale")
			{
				glm::vec3* p = std::any_cast<glm::vec3*>(ComponentProperties::GetValue(prop, pData));
				SetScale({ p->x, p->y, p->z });
			}
		}

		//Getting
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }
		inline const glm::vec3& GetScale() const { return m_Scale; }

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; }
		inline void SetScale(const glm::vec3& s) { m_Scale = s; }

	private:
		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;
	};
}