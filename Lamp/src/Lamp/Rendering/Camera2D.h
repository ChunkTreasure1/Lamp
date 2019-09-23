#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Lamp/Window.h"

namespace Lamp
{
	class Camera2D
	{
	public:
		Camera2D(WindowProps& props = WindowProps());
		~Camera2D();

		//Functions
		void Update();
		glm::vec2 ScreenToWorldCoords(glm::vec2 screenCoords);

		//Setting
		inline void SetPosition(const glm::vec2& newPosition) { m_Position = newPosition; m_MatrixChanged = true; }
		inline void SetScale(float newScale) { m_Scale = newScale; m_MatrixChanged = true; }

		//Getting
		inline const glm::vec2 GetPosition() const { return m_Position; }
		inline const glm::mat4 GetMatrix() const { return m_CameraMatrix; }
		inline const float GetScale() const { return m_Scale; }

	private:

		//Member vars
		glm::vec2 m_Position;
		glm::mat4 m_CameraMatrix;
		glm::mat4 m_OrthoMatrix;

		float m_Scale;
		bool m_MatrixChanged;

		uint32_t m_WindowWidth;
		uint32_t m_WindowHeight;

	};
}