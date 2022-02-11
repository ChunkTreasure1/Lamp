#include "lppch.h"
#include "Commands.h"

#include <Lamp/Objects/Object.h>

namespace Sandbox
{
	TranslateCommand::TranslateCommand(Lamp::Object* object, const glm::mat4& newTransform)
		: m_object(object), m_endTransform(newTransform)
	{
	}

	TranslateCommand::~TranslateCommand()
	{
	}

	void TranslateCommand::Execute()
	{
	}

	void TranslateCommand::Undo()
	{
		if (m_object)
		{
			m_object->SetTransform(m_startTransform);
		}
	}
}