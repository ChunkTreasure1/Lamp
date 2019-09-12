#include "lppch.h"
#include "LayerStack.h"

namespace Lamp
{
	LayerStack::LayerStack()
	{
	}


	LayerStack::~LayerStack()
	{
		for (Layer* pLayer : m_pLayers)
		{
			delete pLayer;
		}
	}

	void LayerStack::PushLayer(Layer * pLayer)
	{
		m_pLayers.emplace(m_pLayers.begin() + m_LayerInsertIndex, pLayer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PopLayer(Layer * pLayer)
	{
		auto it = std::find(m_pLayers.begin(), m_pLayers.end(), pLayer);
		if (it != m_pLayers.end())
		{
			m_pLayers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PushOverlay(Layer * pOverlay)
	{
		//Place it in the back to draw last
		m_pLayers.emplace_back(pOverlay);
	}

	void LayerStack::PopOverlay(Layer * pOverlay)
	{
		auto it = std::find(m_pLayers.begin(), m_pLayers.end(), pOverlay);
		if (it != m_pLayers.end())
		{
			m_pLayers.erase(it);
		}
	}
}