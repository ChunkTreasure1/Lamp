#pragma once
#include "Event.h"

namespace Lamp
{
	class KeyEvent : public Event
	{
	protected:
		KeyEvent(int keyCode)
			: m_KeyCode(keyCode)
		{
		}

		int m_KeyCode
	};
}