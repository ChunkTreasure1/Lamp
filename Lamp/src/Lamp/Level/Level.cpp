#include "lppch.h"
#include "Level.h"

#include "Lamp/Objects/Entity/BaseComponents/CameraComponent.h"

namespace Lamp
{
	void Level::OnEvent(Event& e)
	{
		for (auto& it : m_Entities)
		{
			it.second->OnEvent(e);
		}

		for (auto& it : m_Brushes)
		{
			it.second->OnEvent(e);
		}
	}

	void Level::UpdateEditor(Timestep ts, Ref<CameraBase>& camera)
	{
		RenderPassManager::Get()->RenderPasses(camera);
	}

	void Level::UpdateRuntime(Timestep ts)
	{
		AppUpdateEvent e(ts);
		OnEvent(e);

		Ref<CameraBase> camera;
		for (auto& it : m_Entities)
		{
			if (auto& comp = it.second->GetComponent<CameraComponent>())
			{
				camera = comp->GetCamera();
				break;
			}
		}

		if (camera)
		{
			RenderPassManager::Get()->RenderPasses(camera);
		}
	}
}