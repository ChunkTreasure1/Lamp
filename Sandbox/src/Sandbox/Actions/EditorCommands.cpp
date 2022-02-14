#include "lppch.h"
#include "EditorCommands.h"

#include <Lamp/Level/LevelManager.h>
#include <Lamp/Objects/Object.h>

namespace Sandbox
{
	TransformCommand::TransformCommand(Lamp::Object* object, const glm::mat4& newTransform)
		: m_object(object), m_endTransform(newTransform)
	{
	}

	void TransformCommand::Execute()
	{
	}

	void TransformCommand::Undo()
	{
		if (m_object)
		{
			m_object->SetTransform(m_startTransform);
		}
	}

	SelectCommand::SelectCommand(Lamp::Object* lastSelected, Lamp::Object* newSelected, Lamp::Object** selectedObject)
		: m_lastSelected(lastSelected), m_newSelected(newSelected), m_selectedObject(selectedObject)
	{
	}

	void SelectCommand::Execute()
	{
	}

	void SelectCommand::Undo()
	{
		if (m_selectedObject)
		{
			*m_selectedObject = m_lastSelected;
		}
	}

	DuplicateCommand::DuplicateCommand(Lamp::Object* lastSelected, Lamp::Object* newObj, Lamp::Object** selectedObject)
		: m_lastSelected(lastSelected), m_newObject(newObj), m_selectedObject(selectedObject)
	{
	}

	void DuplicateCommand::Execute()
	{
	}

	void DuplicateCommand::Undo()
	{
		if (m_selectedObject)
		{
			if (Lamp::LevelManager::IsLevelLoaded())
			{
				auto level = Lamp::LevelManager::GetActive();

				m_newObject->Destroy();
				*m_selectedObject = m_lastSelected;
			}
		}
	}
}