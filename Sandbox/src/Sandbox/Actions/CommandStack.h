#pragma once

#include "EditorCommands.h"

#include <Lamp/Core/Core.h>

#include <stack>

namespace Sandbox
{
	class CommandStack
	{
	public:
		void Push(Ref<Command> cmd);
		void Undo();

	private:
		std::stack<Ref<Command>> m_commandStack;
	};
}