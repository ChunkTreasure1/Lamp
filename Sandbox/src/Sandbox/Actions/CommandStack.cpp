#include "lppch.h"
#include "CommandStack.h"

namespace Sandbox
{
	void CommandStack::Push(Ref<Command> cmd)
	{
		m_commandStack.emplace(cmd);
	}

	void CommandStack::Undo()
	{
		if (m_commandStack.empty())
		{
			return;
		}

		m_commandStack.top()->Undo();
		m_commandStack.pop();
	}
}