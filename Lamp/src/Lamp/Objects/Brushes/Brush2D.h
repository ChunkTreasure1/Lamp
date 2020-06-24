#pragma once

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Renderer2D.h"

#include "Lamp/Objects/Object.h"
#include "Lamp/Physics/Colliders/AABB.h"

namespace Lamp
{
	class Brush2D : public Object
	{
	public:
		Brush2D(const std::string& spritePath)
		{
			m_Name = "Unnamned Brush";

			m_PhysicalEntity = std::make_shared<PhysicalEntity>();
			m_PhysicalEntity->SetCollider(std::make_shared<AABB>(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f)));

			m_Sprite = Texture2D::Create(spritePath);
		}

		virtual void OnEvent(Event& e) override
		{
			if (e.GetEventType() == EventType::AppRender)
			{
				OnRender();
			}
		}

	private:
		bool OnRender()
		{
			Renderer2D::DrawQuad(m_ModelMatrix, m_Sprite);

			return true;
		}

	private:
		bool m_ShouldCollide;
		std::string m_SpritePath;
		Ref<Texture2D> m_Sprite;
	};
}