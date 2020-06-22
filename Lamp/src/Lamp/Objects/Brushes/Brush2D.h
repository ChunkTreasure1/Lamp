#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Renderer2D.h"

namespace Lamp
{
	class Brush2D
	{
	public:
		Brush2D(const std::string& spritePath)
			: m_SpritePath(spritePath), m_Position(0, 0, 0), m_Rotation(0, 0, 0), m_Scale(1, 1, 1), m_TransformMatrix(1.f)
		{
			m_Sprite = Texture2D::Create(spritePath);
		}

		void Draw()
		{
			Renderer2D::DrawQuad(m_TransformMatrix, m_Sprite);
		}

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; RecalculateMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; RecalculateMatrix(); }
		inline void SetScale(const glm::vec3& scale) { m_Scale = scale; RecalculateMatrix(); }

		inline void SetShouldCollider(bool state) { m_ShouldCollide = state; }

		//Getting
		inline const glm::vec3& GetPosition() { return m_Position; }
		inline const glm::vec3& GetRotation() { return m_Rotation; }
		inline const glm::vec3& GetScale() { return m_Scale; }

		inline const bool GetShouldCollide() { return m_ShouldCollide; }
		inline const std::string& GetTexturePath() { return m_SpritePath; }
		inline const glm::mat4& GetTM() { return m_TransformMatrix; }

	private:
		void RecalculateMatrix()
		{
			m_TransformMatrix = glm::translate(glm::mat4(1.f), m_Position)
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.x), glm::vec3(1.f, 0.f, 0.f))
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.y), glm::vec3(0.f, 1.f, 0.f))
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.z), glm::vec3(0.f, 0.f, 1.f))
				* glm::scale(glm::mat4(1.f), m_Scale);
		}

	private:
		bool m_ShouldCollide;
		std::string m_SpritePath;
		Ref<Texture2D> m_Sprite;

		glm::mat4 m_TransformMatrix;
		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;
	};
}