#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lamp
{
	class Camera2D
	{
	public:
		Camera2D();
		~Camera2D();

		//Functions
		void Update();
		void Initialize(int windowWidth, int windowHeight);
		glm::vec2 ScreenToWorldCoords(glm::vec2 screenCoords);

		//Setting
		void SetPosition(const glm::vec2& newPosition) { m_Position = newPosition; m_MatrixChanged = true; }
		void SetScale(float newScale) { m_Scale = newScale; m_MatrixChanged = true; }

		//Getting
		glm::vec2 GetPosition() { return m_Position; }
		glm::mat4 GetMatrix() { return m_CameraMatrix; }
		float GetScale() { return m_Scale; }

	private:

		//Member vars
		glm::vec2 m_Position;
		glm::mat4 m_CameraMatrix;
		glm::mat4 m_OrthoMatrix;

		float m_Scale;
		bool m_MatrixChanged;

		int m_WindowWidth;
		int m_WindowHeight;

	};
}