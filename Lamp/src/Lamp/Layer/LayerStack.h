#pragma once
#include "lppch.h"

#include "Layer.h"

namespace Lamp
{
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		void PushOverlay(Layer* overlay);
		void PopOverlay(Layer* overlay);

		std::vector<Layer*>::iterator begin() { return m_pLayers.begin(); }
		std::vector<Layer*>::iterator end() { return m_pLayers.end(); }

	private:
		std::vector<Layer*> m_pLayers;
		unsigned int m_LayerInsertIndex = 0;
	};
}