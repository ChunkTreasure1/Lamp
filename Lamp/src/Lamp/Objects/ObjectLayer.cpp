#include "lppch.h"
#include "ObjectLayer.h"

namespace Lamp
{
	Ref<ObjectLayerManager> ObjectLayerManager::s_ObjectLayerManager = nullptr;
	void ObjectLayerManager::OnEvent(Event& e)
	{
		for (auto& l : m_Layers)
		{
			for (int i = 0; i < l.Objects.size(); i++)
			{
				l.Objects[i]->OnEvent(e);
			}
		}
	}


}