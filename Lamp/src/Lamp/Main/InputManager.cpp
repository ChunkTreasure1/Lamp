#include "InputManager.h"

namespace CactusEngine
{
	InputManager::InputManager() :
		m_MouseCoords(0.f)
	{
	}

	InputManager::~InputManager()
	{
	}

	//Called when a key is pressed
	void InputManager::KeyPressed(unsigned int keyID)
	{
		m_KeyMap[keyID] = true;
	}

	//Called when a key is released
	void InputManager::KeyReleased(unsigned int KeyID)
	{
		m_KeyMap[KeyID] = false;
	}

	//Returns the state of the key
	bool InputManager::IsKeyPressed(unsigned int keyID)
	{
		auto it = m_KeyMap.find(keyID);

		if (it != m_KeyMap.end())
		{
			return it->second;
		}
		else
		{
			return false;
		}
	}

	//Sets the coordinates of the mouse
	void InputManager::SetMouseCoords(float x, float y)
	{
		m_MouseCoords.x = x;
		m_MouseCoords.y = y;
	}
}