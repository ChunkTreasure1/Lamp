#pragma once

#include "Commands.h"

#include <stack>

namespace Sandbox
{
	class CommandStack
	{
	public:
		void Undo();

	private:
		std::stack<Ref<Command>> m_commandStack;
	};
}