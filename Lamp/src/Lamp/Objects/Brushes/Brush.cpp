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
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Brush::OnRender));
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

	bool Brush::OnRender(AppRenderEvent& e)
	{
		m_Model->Render(m_Id);

		if (g_pEnv->ShouldRenderBB && !e.GetPassInfo().IsShadowPass && !e.GetPassInfo().IsPointShadowPass)
		{
			m_Model->RenderBoundingBox();
		}

		return true;
	}
}