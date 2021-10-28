#pragma once

#include "BaseWindow.h"

namespace Sandbox
{
	class LayerViewPanel : public BaseWindow
	{
	public:
		LayerViewPanel(std::string_view name);

		virtual void OnEvent(Lamp::Event& e) override;

		inline Lamp::Object* GetSelectedObject() { return m_pSelectedObject; }

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool Update(Lamp::AppUpdateEvent& e);

	private:
		Lamp::Object* m_pSelectedObject = nullptr;
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