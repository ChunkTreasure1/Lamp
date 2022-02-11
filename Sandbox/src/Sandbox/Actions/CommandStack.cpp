#include "lppch.h"
#include "CommandStack.h"

namespace Sandbox
{
	void CommandStack::Undo()
	{
		m_commandStack.top()->Undo();
		m_commandStack.pop();
	}
}