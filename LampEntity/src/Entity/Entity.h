#pragma once

#include <vector>
#include <memory>
#include <Lamp/Core/Timestep.h>

#include "Component/EntityComponent.h"

namespace LampEntity
{
	class Entity
	{
	public:
		void Update(Lamp::Timestep ts);
		void Draw();

		//Getting
		inline const bool GetIsActive() const { return m_IsActive; }
		
		template<typename T>
		T* GetComponent();

		template<typename T, typename... TArgs>
		T& AddComponent(TArgs&&... mArgs);

		//Setting
		inline void SetIsActive(bool state) { m_IsActive = state; }

	private:
		bool m_IsActive = true;
		std::vector<std::unique_ptr<IEntityComponent>> m_pComponents;

		ComponentArray m_ComponentArray;
		ComponentBitSet m_ComponentBitSet;
	};

	template<typename T>
}