#pragma once

#include "EditorWindow.h"

#include <Lamp/Event/ApplicationEvent.h>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           >
#include <Lamp/Rendering/Textures/Texture2D.h>

namespace Sandbox
{
	class LayerViewPanel : public EditorWindow
	{
	public:
		LayerViewPanel(std::string_view name, Lamp::Object** selectedObject);
		void OnEvent(Lamp::Event& e) override;

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool Update(Lamp::AppUpdateEvent& e);

	private:
		Lamp::Object** m_pSelectedObject = nullptr;
		int m_renamingLayer = -1;
		int m_lastRenamingLayer = -1;

		Ref<Lamp::Texture2D> m_entityIcon;
		Ref<Lamp::Texture2D> m_brushIcon;

		Ref<Lamp::Texture2D> m_visibleIconV;
		Ref<Lamp::Texture2D> m_visibleIconN;
		
		Ref<Lamp::Texture2D> m_lockedIcon;
		Ref<Lamp::Texture2D> m_unlockedIcon;
	};
}