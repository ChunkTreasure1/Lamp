#pragma once

#include <string>
#include <glm/glm.hpp>
#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Renderer2D.h"

namespace Lamp
{
	class Brush2D
	{
	public:
		Brush2D(const std::string& spritePath)
			: m_SpritePath(spritePath), m_Position(0, 0, 0), m_Rotation(0, 0, 0), m_Scale(1, 1, 1)
		{
			m_Sprite = Texture2D::Create(spritePath);
		}

		void Draw()
		{
			Renderer2D::DrawQuad(m_Position, m_Scale, m_Rotation.x, m_Sprite);
		}

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; }
		inline void SetScale(const glm::vec3& scale) { m_Scale = scale; }

		inline void SetShouldCollider(bool state) { m_ShouldCollide = state; }

		//Getting
		inline const glm::vec3& GetPosition() { return m_Position; }
		inline const glm::vec3& GetRotation() { return m_Rotation; }
		inline const glm::vec3& GetScale() { return m_Scale; }

		inline const bool GetShouldCollide() { return m_ShouldCollide; }
		bool m_ShouldCollide;
		inline const std::string& GetTexturePath() { return m_SpritePath; }

	private:
		std::string m_SpritePath;
		Ref<Texture2D> m_Sprite;


		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;
	};
}