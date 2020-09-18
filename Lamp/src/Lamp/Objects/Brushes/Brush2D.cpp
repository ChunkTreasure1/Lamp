#include "lppch.h"
#include "Brush2D.h"

#include "BrushManager.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	Brush2D::Brush2D(const std::string& spritePath)
	{
		m_Name = "Unnamned Brush";
		SAABB bb;
		bb.Min = glm::vec3(-0.5f, -0.5f, -0.5f);
		bb.Max = glm::vec3(0.5f, 0.5f, 0.5f);

		m_PhysicalEntity = std::make_shared<PhysicalEntity>();
		m_PhysicalEntity->SetCollider(std::make_shared<AABB>(bb, m_Position));

		m_Sprite = Texture2D::Create(spritePath);
	}

	void Brush2D::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Brush2D::OnRender));
	}

	void Brush2D::Destroy()
	{
		BrushManager::Get()->Remove(this);
		ObjectLayerManager::Get()->Remove(this);

		delete this;
	}

	Brush2D* Brush2D::Create(const std::string& path)
	{
		return BrushManager::Get()->Create2D(path);
	}

	bool Brush2D::OnRender(AppRenderEvent& e)
	{
		Renderer2D::DrawQuad(m_ModelMatrix, m_Sprite);
		m_PhysicalEntity->GetCollider()->Render();

		return true;
	}

}