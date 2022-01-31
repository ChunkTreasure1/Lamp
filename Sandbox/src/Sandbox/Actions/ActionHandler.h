#pragma once

#include <any>
#include <glm/glm.hpp>
#include <list>

#include <Lamp/Objects/Object.h>

namespace Sandbox
{
	enum class ActionType
	{
		Transform,
		Add,
		Remove,
		Select
	};

	struct Action
	{
		virtual void Transform() {};

		std::any data;
		std::any lastData;

		void* pObject = nullptr;
		ActionType type;
	};

	struct TransformAction : public Action
	{
		TransformAction()
		{
			type = ActionType::Transform;
		}

		virtual void Transform() override
		{
			Lamp::Object* pObj = static_cast<Lamp::Object*>(pObject);
			if (pObj)
			{
				glm::mat4 mat = std::any_cast<glm::mat4>(lastData);
				pObj->SetTransform(mat);
			}
		}
	};

	class ActionHandler
	{
	public:
		void Undo();
		void Redo();

		void AddAction(Action& action);

	private:
		std::list<Action> m_Actions;
	};
}