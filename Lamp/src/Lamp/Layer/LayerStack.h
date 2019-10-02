#pragma once

#include "Lamp/Core/Core.h"
#include "Layer.h"

#include <vector>

namespace Lamp
{
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		std::vector<Layer*>::iterator begin() { return m_pLayers.begin(); }
		std::vector<Layer*>::iterator end() { return m_pLayers.end(); }
	private:
		std::vector<Layer*> m_pLayers;
		uint32_t m_LastInsertIndex = 0;
	};
}