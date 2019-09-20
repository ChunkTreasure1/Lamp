#pragma once

#include <unordered_map>
#include <glm/glm.hpp>

namespace CactusEngine
{
	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		//Functions
		void KeyPressed(unsigned int keyID);
		void KeyReleased(unsigned int KeyID);
		bool IsKeyPressed(unsigned int keyID);

		void SetMouseCoords(float x, float y);
		glm::vec2 GetMouseCoords() const { return m_MouseCoords; }

	private:
		std::unordered_map<unsigned int, bool> m_KeyMap;
		glm::vec2 m_MouseCoords;
	};
}