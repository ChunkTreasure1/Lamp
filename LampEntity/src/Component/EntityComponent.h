#pragma once

#include <Lamp/Event/Event.h>

namespace LampEntity
{
	class IEntityComponent
	{
	public:
		virtual ~IEntityComponent() = 0;

		virtual void Update(Lamp::Event& event) = 0;
		virtual void OnEvent() = 0;

	protected:
		IEntityComponent() {};
	};
}