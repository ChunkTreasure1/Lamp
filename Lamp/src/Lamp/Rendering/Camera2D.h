#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	class Camera2D
	{
	public:
		Camera2D(uint32_t width, uint32_t height);
		~Camera2D();

		void Update();
		void Initialize(uint32_t width, uint32_t height);
		glm::vec2 ScreenToWorldCoords(glm::vec2 screenCoords);

		//Setting
		inline void SetPosition(const glm::vec2& pos) { m_Position = pos; m_MatrixChanged = true; }
		inline void SetScale(float scale) { m_Scale = scale; m_MatrixChanged = true; }

		//Getting
		inline const glm::vec2 GetPosition() const { return m_Position; }
		inline const glm::mat4 GetMatrix() const { return m_CameraMatrix; }
		inline const float GetScale() const { return m_Scale; }


	private:
		glm::vec2 m_Position;
		glm::mat4 m_CameraMatrix;
		glm::mat4 m_OrthoMatrix;

		float m_Scale;
		bool m_MatrixChanged = true;

		uint32_t m_WindowWidth;
		uint32_t m_WindowHeight;
	};
}