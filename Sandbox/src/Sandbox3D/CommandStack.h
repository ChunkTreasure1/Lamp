#pragma once

#include <list>

namespace Sandbox3D
{
	enum class CommandType
	{
		Position,
		Rotation,
		Scale,
		Transform,
		Selection
	};

	struct Command
	{
		CommandType commandType;
		void* lastData = nullptr;
		void* object = nullptr;
	};

	template<typename T>
	class CommandStack
	{
	public:
		CommandStack(uint32_t maxCmds)
			: m_MaxCommands(maxCmds)
		{
		}

		T& front()
		{
			return m_Undos.front();
		}

		T& back()
		{
			return m_Undos.back();
		}

		T& redo_top()
		{
			return m_Redos.front();
		}

		void redo_pop()
		{
			m_Redos.pop_front();
		}

		void pop_front()
		{
			m_Redos.push_front(std::move(m_Undos.front()));
			m_Undos.pop_front();
		}

		void pop_back()
		{
			m_Undos.pop_back();
		}

		void push_front(T& e)
		{
			if (m_Undos.size() == m_MaxCommands)
			{
				m_Undos.pop_back();
			}
			m_Undos.push_front(std::move(e));
			m_Redos.clear();
		}

		void clear()
		{
			m_Undos.clear();
			m_Redos.clear();
		}

		bool empty()
		{
			return m_Undos.empty();
		}

		bool redo_empty()
		{
			return m_Redos.empty();
		}

		inline void SetMaxCommands(uint32_t cmds)
		{
			m_MaxCommands = cmds;
		}

	private:
		std::list<T> m_Undos;
		std::list<T> m_Redos;
		uint32_t m_MaxCommands;
	};
}