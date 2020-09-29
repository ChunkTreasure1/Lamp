#include "lppch.h"
#include "Brush.h"

#include "BrushManager.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	void Brush::UpdatedMatrix()
	{
		m_Model->SetModelMatrix(m_ModelMatrix);
	}

	void Brush::OnEvent(Event& e)
	{
		if (e.GetEventType() == EventType::AppRender)
		{
			OnRender();
		}
	}

	void Brush::Destroy()
	{
		BrushManager::Get()->Remove(this);
		ObjectLayerManager::Get()->Remove(this);

		delete this;
	}

	Brush* Brush::Create(const std::string& path)
	{
		return BrushManager::Get()->Create(path);
	}

	bool Brush::OnRender()
	{
		m_Model->Render();

		if (g_pEnv->ShouldRenderBB)
		{
			m_Model->RenderBoundingBox();
		}

		return true;
	}
}