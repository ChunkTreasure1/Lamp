#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Meshes/Model.h"
#include "Lamp/Objects/Entity/Base/Physical/PhysicalEntity.h"

#include "Lamp/Objects/Object.h"

namespace Lamp
{
	class Brush : public Object
	{
	public:
		Brush(Ref<Model> model)
			: m_Model(model)
		{
			m_pPhysicalEntity = std::make_shared<PhysicalEntity>();
			m_pPhysicalEntity->SetCollider(std::make_shared<BoundingSphere>(m_Position, 1.f));
		}

		void Render()
		{
			m_Model->Render();
		}

		virtual void UpdatedMatrix() override
		{
			m_Model->SetModelMatrix(m_ModelMatrix);
		}

		//Getting
		inline Ref<Model>& GetModel() { return m_Model; }

	private:
		Ref<Model> m_Model;
	};
}