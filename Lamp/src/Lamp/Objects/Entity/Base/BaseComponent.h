#pragma once

#include <array>
#include <bitset>
#include "ComponentProperties.h"
#include "Lamp/Event/Event.h"

#include "Lamp/Core/Time/Timestep.h"

namespace Lamp
{
	class Entity;
	class EntityComponent
	{
	public:

		//Setting
		inline void MakeOwner(Entity* pEntity) { m_pEntity = pEntity; }
		inline void SetComponentProperties(const ComponentProperties& properties) { m_ComponentProperties = properties; }

		//Getting
		inline Entity* GetOwner() { return m_pEntity; }
		inline ComponentProperties& GetComponentProperties() { return m_ComponentProperties; }
		inline const std::string& GetName() { return m_Name; }

		//Base
		virtual void Initialize() {}
		virtual void OnEvent(Event& e) {}
		virtual uint64_t GetEventMask() = 0;

	protected:
		EntityComponent(std::string name)
			: m_pEntity(nullptr), m_ComponentProperties({}), m_Name(name)
		{}
		Entity* m_pEntity;
		ComponentProperties m_ComponentProperties;
		std::string m_Name;
	};
}