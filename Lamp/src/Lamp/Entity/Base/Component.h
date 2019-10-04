#pragma once

#include <array>
#include <bitset>

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

	struct EditorValues
	{
		EditorValues(const std::string& name)
			: Name(name)
		{ }

		std::string Name;
	};

	class IEntityComponent
	{
	public:

		//Setting
		inline void MakeOwner(IEntity* pEntity) { m_pEntity = pEntity; }

		//Getting
		inline const IEntity* GetOwner() const { return m_pEntity; }
		virtual const EditorValues GetEditorValues() const = 0;

		//Base
		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;

	protected:
		IEntityComponent() 
		{}
		IEntity* m_pEntity;
	};
}