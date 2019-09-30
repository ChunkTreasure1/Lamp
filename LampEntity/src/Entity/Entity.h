#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "Component/EntityComponent.h"

namespace LampEntity
{
	class IEntity
	{
	public:
		virtual ~IEntity();
		void Update();
		void Draw();

		//Getting
		inline const bool GetIsActive() const { return m_IsActive; }
	
		//Setting
		inline void SetIsActive(bool state) { m_IsActive = state; }

		template<typename T>
		T* GetComponent();

		template<typename T, typename... TArgs>
		T& GetOrCreateComponent(TArgs&&... mArgs);

		template<typename T>
		bool RemoveComponent();

	protected:
		IEntity() {}

		//Editor
		glm::mat2x2 m_TransformMatrix;
		glm::mat2x2 m_RotationMatrix;
		glm::mat2x2 m_ScaleMatrix;

	private:
		bool m_IsActive = true;
		std::vector<std::unique_ptr<IEntityComponent>> m_pComponents;

		ComponentArray m_ComponentArray;
		ComponentBitSet m_ComponentBitSet;
	};
}