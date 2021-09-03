#include "ActionHandler.h"

namespace Sandbox3D
{
	void ActionHandler::Undo()
	{
		switch (m_Actions.front().type)
		{
			case ActionType::Add:
				break;

			case ActionType::Remove:
				break;

			case ActionType::Select:
				break;

			case ActionType::Transform:
				m_Actions.front().Transform();
				break;

			m_Actions.pop_front();
		}
	}

	void ActionHandler::Redo()
	{
	}

	void ActionHandler::AddAction(Action& action)
	{
		m_Actions.push_front(action);
	}
}