#pragma once

#include <array>
#include <bitset>

#include <Lamp/Event/Event.h>

namespace LampEntity
{
	class IEntity;
	class IEntityComponent;

	using ComponentID = std::size_t;

	inline ComponentID GetComponentTypeID()
	{
		static ComponentID lastID = 0;
		return lastID++;
	}

	template<class T> 
	inline ComponentID GetComponentTypeID() noexcept
	{
		static ComponentID typeID = GetComponentTypeID();
		return typeID;
	}

	constexpr std::size_t maxComponents = 32;

	using ComponentBitSet = std::bitset<maxComponents>;
	using ComponentArray = std::array<IEntityComponent*, maxComponents>;

	class IEntityComponent
	{
	public:
		virtual ~IEntityComponent() = 0;

		//Setting
		inline void MakeOwner(IEntity* pEntity) { m_pEntity = pEntity; }

		//Getting
		inline const IEntity* GetEntity() const { return m_pEntity; }

		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void OnEvent(Lamp::Event& event) = 0;
		virtual void Draw() = 0;

	protected:
		IEntityComponent() {};

		IEntity* m_pEntity;
	};
}