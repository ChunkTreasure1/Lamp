#pragma once

#include <array>
#include <bitset>
#include "ComponentProperties.h"

namespace Lamp
{
	class IEntityComponent;
	class IEntity;

	using ComponentID = std::uint32_t;

	inline ComponentID GetComponentTypeID()
	{
		static ComponentID lastID = 0;
		return lastID++;
	}

	template<typename T>
	inline ComponentID GetComponentTypeID() noexcept
	{
		static ComponentID typeID = GetComponentTypeID();
		return typeID;
	}

	using ComponentBitSet = std::bitset<32>;
	using ComponentArray = std::array<IEntityComponent*, 32>;

	class IEntityComponent
	{
	public:

		//Setting
		inline void MakeOwner(IEntity* pEntity) { m_pEntity = pEntity; }
		inline void SetComponentProperties(const ComponentProperties& properties) { m_ComponentProperties = properties; }

		//Getting
		inline const IEntity* GetOwner() const { return m_pEntity; }
		inline ComponentProperties& GetComponentProperties() { return m_ComponentProperties; }
		inline const std::string& GetName() { return m_Name; }

		//Base
		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;
		virtual void SetProperty(ComponentProperty& prop, void* pData) = 0;
	protected:
		IEntityComponent(std::string name)
			: m_pEntity(nullptr), m_ComponentProperties({}), m_Name(name)
		{}
		IEntity* m_pEntity;
		ComponentProperties m_ComponentProperties;
		std::string m_Name;
	};
}