#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	class Object;
}

namespace Sandbox
{
	class Command
	{
	public:
		virtual ~Command() {}

		virtual void Execute() = 0;
		virtual void Undo() = 0;
	};

	class TransformCommand : public Command
	{
	public:
		TransformCommand(Lamp::Object* object, const glm::mat4& newTransform);
		~TransformCommand() override = default;

		void Execute() override;
		void Undo() override;

	private:
		glm::mat4 m_startTransform;
		glm::mat4 m_endTransform;
		Lamp::Object* m_object;
	};

	class SelectCommand : public Command
	{
	public:
		SelectCommand(Lamp::Object* lastSelected, Lamp::Object* newSelected, Lamp::Object** selectedObject);
		~SelectCommand() override = default;

		void Execute() override;
		void Undo() override;

	private:
		Lamp::Object* m_lastSelected;
		Lamp::Object* m_newSelected;
		Lamp::Object** m_selectedObject;
	};

	class DuplicateCommand : public Command
	{
	public:
		DuplicateCommand(Lamp::Object* lastSelected, Lamp::Object* newObj, Lamp::Object** selectedObject);
		~DuplicateCommand() override = default;

		void Execute() override;
		void Undo() override;

	private:
		Lamp::Object* m_lastSelected;
		Lamp::Object* m_newObject;
		Lamp::Object** m_selectedObject;
	};
}