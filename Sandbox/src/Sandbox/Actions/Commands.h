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

	class TranslateCommand : public Command
	{
	public:
		TranslateCommand(Lamp::Object* object, const glm::mat4& newTransform);
		~TranslateCommand() override;

		void Execute() override;
		void Undo() override;

	private:
		glm::mat4 m_startTransform;
		glm::mat4 m_endTransform;
		Lamp::Object* m_object;
	};
}