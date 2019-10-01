#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "LampEntity/Component/EntityComponent.h"

namespace LampEntity
{
	class IEntity
	{
	public:
		IEntity() {}
		~IEntity() {};
		void Update();
		void Draw() {};

		//Getting
		inline const bool GetIsActive() const { return m_IsActive; }

		//Setting
		inline void SetIsActive(bool state) { m_IsActive = state; }

		template<typename T>
		T* GetComponent()
		{
			if (m_ComponentBitSet[GetComponentTypeID<T>()])
			{
				return m_ComponentArray[GetComponentTypeID<T>()];
			}
			else
			{
				return nullptr;
			}
		}

		template<typename T, typename... TArgs>
		T* GetOrCreateComponent(TArgs&&... mArgs)
		{
			if (m_ComponentBitSet[GetComponentTypeID<T>()])
			{
				T* c(new T(std::forward<TArgs>(mArgs)...));
				c->MakeOwner(this);
				std::unique_ptr<IEntityComponent> uPtr { c };
				m_pComponents.emplace_back(std::move(uPtr));

				m_ComponentArray[GetComponentTypeID<T>()] = c;
				m_ComponentBitSet[GetComponentTypeID<T>()] = true;
			
				c->Initialize();

				return c;
			}
			else
			{
				return (T*)m_ComponentArray[GetComponentTypeID<T>()];
			}
		}

		template<typename T>
		bool RemoveComponent()
		{
			//if (m_ComponentBitSet[GetComponentTypeID< T >()])
			//{
			//	for (size_t i = 0; i < m_pComponents.size(); i++)
			//	{
			//		if (m_pComponents[i] == m_ComponentArray[GetComponentTypeID< T >()])
			//		{
			//			m_pComponents.erase(m_pComponents.begin() + i);
			//			break;
			//		}
			//	}

			//	m_ComponentArray[GetComponentTypeID< T >()] = nullptr;
			//	m_ComponentBitSet[GetComponentTypeID< T >()] = false;
			//}
			//else
			//{
			//	return false;
			//}

			return false;
		}

	private:
		//Editor
		glm::mat2x2 m_TransformMatrix;
		glm::mat2x2 m_RotationMatrix;
		glm::mat2x2 m_ScaleMatrix;

		bool m_IsActive = true;
		std::vector<IEntityComponent*> m_pComponents;

		ComponentArray m_ComponentArray;
		ComponentBitSet m_ComponentBitSet;
	};
}