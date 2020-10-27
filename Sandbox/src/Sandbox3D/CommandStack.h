#pragma once

#include <list>

namespace Sandbox3D
{
	enum class Cmd
	{
		Position,
		Rotation,
		Scale,
		Transform,
		Selection
	};

	struct Command
	{
		Cmd cmd;
		void* lastData = nullptr;
		void* object = nullptr;
	};

	template<typename T>
	class CommandStack
	{
	public:
		CommandStack(uint32_t maxCmds)
			: m_Elements(maxCmds)
		{
		}

		T& front()
		{
			return m_Elements.front();
		}

		T& back()
		{
			return m_Elements.back();
		}

		void pop_front()
		{
			m_Elements.pop_front();
		}

		void pop_back()
		{
			m_Elements.pop_back();
		}

		void push_front(T& e)
		{
			if (m_Elements.size() == m_MaxCommands)
			{
				m_Elements.pop_back();
			}
			m_Elements.push_front(std::move(e));
		}

		void clear()
		{
			m_Elements.clear();
		}

		bool empty()
		{
			return m_Elements.empty();
		}

		inline void SetMaxCommands(uint32_t cmds)
		{
			m_MaxCommands = cmds;
		}

	private:
		std::list<T> m_Elements;
		uint32_t m_MaxCommands;
	};
}