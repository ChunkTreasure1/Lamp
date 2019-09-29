#pragma once

#include <array>
#include <bitset>

#include <Lamp/Event/Event.h>

namespace LampEntity
{
	class IEntityComponent;

	using ComponentID = std::size_t;

	inline ComponentID GetComponentTypeID()
	{
		static ComponentID lastID = 0;
		return lastID++;
	}

	template<typename T> 
	inline ComponentID GetComponentTypeID() noexcept
	{
		static ComponentID typeID = GetComponentTypeID();
		return typeID();
	}

	constexpr std::size_t maxComponents = 32;

	using ComponentBitSet = std::bitset<maxComponents>;
	using ComponentArray = std::array<IEntityComponent*, maxComponents>;

	class IEntityComponent
	{
	public:
		virtual ~IEntityComponent() = 0;

		//Setting
		inline void MakeOwner(Entity* pEntity) { m_pEntity = pEntity; }

		//Getting
		inline const Entity* GetEntity() const { return m_pEntity; }

		virtual void Initialize() = 0;
		virtual void Update(Lamp::Timestep ts) = 0;
		virtual void OnEvent(Lamp::Event& event) = 0;

	protected:
		IEntityComponent() {};

		Entity* m_pEntity;
	};
}