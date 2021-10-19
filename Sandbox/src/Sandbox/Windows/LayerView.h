#pragma once

#include "BaseWindow.h"

namespace Sandbox
{
	class LayerView : public BaseWindow
	{
	public:
		LayerView(std::string_view name);

		virtual void OnEvent(Lamp::Event& e) override;

		inline Lamp::Object* GetSelectedObject() { return m_pSelectedObject; }

	private:
		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);

		void UpdateMainClickMenu();
		void UpdateItemClickMenu(const std::string& id, bool& open, uint32_t& layerId);

		void CollapsingHeaderAddons(std::string& currRightClick, const std::string& id, bool& open, uint32_t& layerId, const uint32_t& currId);

	private:
		Lamp::Object* m_pSelectedObject = nullptr;
		Ref<Lamp::Texture2D> m_EntityIcon;
		Ref<Lamp::Texture2D> m_BrushIcon;
	};
}