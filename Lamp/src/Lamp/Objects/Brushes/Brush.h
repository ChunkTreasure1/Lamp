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
			m_PhysicalEntity = std::make_shared<PhysicalEntity>();
			m_PhysicalEntity->SetCollider(std::make_shared<BoundingSphere>(m_Position, 1.f));
			m_Name = "Brush";
		}
		bool OnRender()
		{
			m_Model->Render();

			return true;
		}

		virtual void UpdatedMatrix() override
		{
			m_Model->SetModelMatrix(m_ModelMatrix);
		}
		virtual void OnEvent(Event& e) override
		{
			if (e.GetEventType() == EventType::AppRender)
			{
				OnRender();
			}
		}
		//Getting
		inline Ref<Model>& GetModel() { return m_Model; }

	private:
		Ref<Model> m_Model;
	};
}