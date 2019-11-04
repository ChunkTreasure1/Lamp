#pragma once

#include <glm/glm.hpp>

#include "Lamp/Entity/Base/Component.h"

namespace Lamp
{
	class TransformComponent : public IEntityComponent
	{
	public:
		TransformComponent(const glm::vec3& pos)
			: m_Scale(glm::vec3(1.f, 1.f, 1.f)), m_Rotation(glm::vec3(0, 0, 0))
		{
			m_Position = pos;
		}

		virtual void Initialize() override {}
		virtual void Update() override {}
		virtual void Draw() override {}

		//Getting
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }
		inline const glm::vec3& GetScale() const { return m_Scale; }
		virtual const EditorValues GetEditorValues() const { return m_TransformValues; }

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; }
		inline void SetScale(const glm::vec3& s) { m_Scale = s; }

	private:
		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		struct TransformValues : EditorValues
		{
			TransformValues()
				: EditorValues("Transform Component")
			{}
		};

		TransformValues m_TransformValues = TransformValues();
	};
}