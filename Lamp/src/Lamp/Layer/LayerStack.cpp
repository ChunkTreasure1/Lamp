#include "lppch.h"
#include "LayerStack.h"

namespace Lamp
{
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		for (Layer* layer : m_pLayers)
		{
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer * layer)
	{
		m_pLayers.emplace(m_pLayers.begin() + m_LastInsertIndex, layer);
		m_LastInsertIndex++;
		layer->OnAttach();
	}

	void LayerStack::PushOverlay(Layer * overlay)
	{
		m_pLayers.emplace_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(Layer * layer)
	{
		auto it = std::find(m_pLayers.begin(), m_pLayers.begin() + m_LastInsertIndex, layer);
		if (it != m_pLayers.begin() + m_LastInsertIndex)
		{
			layer->OnDetach();
			m_pLayers.erase(it);
			m_LastInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(Layer * overlay)
	{
		auto it = std::find(m_pLayers.begin() + m_LastInsertIndex, m_pLayers.end(), overlay);
		if (it != m_pLayers.end())
		{
			overlay->OnDetach();
			m_pLayers.erase(it);
		}
	}
}