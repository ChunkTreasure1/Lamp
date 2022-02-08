#pragma once

#include <array>
#include <bitset>
#include "ComponentProperties.h"
#include "Lamp/Event/Event.h"

#include "Lamp/Core/Time/Timestep.h"

namespace Lamp
{
	class Entity;
	class LevelSystem;

	class EntityComponent
	{
	public:
		virtual ~EntityComponent() {};
		virtual void Initialize() {}
		virtual void OnEvent(Event& e) {}
		virtual void SetComponentProperties() {}

		inline Entity* GetEntity() { return m_pEntity; }
		inline ComponentProperties& GetComponentProperties() { return m_componentProperties; }
		inline const std::string& GetName() { return m_name; }

	protected:
		EntityComponent(std::string name);

		Entity* m_pEntity;
		ComponentProperties m_componentProperties;
		std::string m_name;

	private:
		friend class Entity;
		friend class LevelSystem;
		friend class LevelLoader;
	};
}