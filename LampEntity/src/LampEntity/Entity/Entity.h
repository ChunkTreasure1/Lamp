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
			if (m_ComponentBitSet[GetComponentTypeID() < T > ])
			{
				return m_ComponentArray[GetComponentTypeID() < T > ];
			}
			else
			{
				return nullptr;
			}
		}

		template<class T>
		T* GetOrCreateComponent()
		{
			if (m_ComponentBitSet[GetComponentTypeID() < T > ])
			{
				T* c(new T());

				c->MakeOwner(this);
				std::unique_ptr<IEntityComponent> uPtr = std::make_unique(c);
				m_pComponents.emplace_back(std::move(uPtr));

				m_ComponentArray[GetComponentTypeID() < T > ] = c;
				m_ComponentBitSet[GetComponentTypeID() < T > ] = true;
			
				c->Initialize();

				return c;
			}
			else
			{
				return m_ComponentArray[GetComponentTypeID() < T > ];
			}

		}

		template<typename T>
		bool RemoveComponent()
		{
			//auto p = T;

			//if (m_ComponentBitSet[GetComponentTypeID() < p > ])
			//{
			//	for (size_t i = 0; i < m_pComponents.size(); i++)
			//	{
			//		if (m_pComponents[i] == m_ComponentArray[GetComponentTypeID() < p > ])
			//		{
			//			m_pComponents.erase(m_pComponents.begin() + i);
			//			break;
			//		}
			//	}

			//	m_ComponentArray[GetComponentTypeID() < p > ] = nullptr;
			//	m_ComponentBitSet[GetComponentTypeID() < p > ] = false;
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
		std::vector<std::unique_ptr<IEntityComponent>> m_pComponents;

		ComponentArray m_ComponentArray;
		ComponentBitSet m_ComponentBitSet;
	};
}