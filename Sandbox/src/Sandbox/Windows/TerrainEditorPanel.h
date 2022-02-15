#pragma once

#include "EditorWindow.h"

#include <Lamp/Event/ApplicationEvent.h>

namespace Sandbox
{
	class TerrainEditorPanel : public EditorWindow
	{
	public:
		TerrainEditorPanel(std::string_view name);

		void OnEvent(Lamp::Event& e) override;

	private:
		enum class EditMode
		{
			Raise,
			Lower,
			Smooth
		};
		
		struct EditParameters
		{
			float innerRadius = 0.f;
			float outerRadius = 1.f;
			EditMode currentEditMode = EditMode::Raise;
		};

		bool OnUpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);

		std::vector<uint32_t> GenerateEmptyTextureData(uint32_t resolution);
		uint32_t CalculateTerrainSize();
		uint32_t GetHeightMapResolution();
		float GetMetersPerPixel();

		std::vector<const char*> m_perPixelSize = { "0.5 meter", "1 meter", "1.5 meters", "2 meters" };
		std::vector<const char*> m_resolutions = { "1024x1024", "2048x2048", "4096x4096" };
	
		int m_selectedSize = 0;
		int m_selectedResolution = 0;
	
		const uint32_t m_minResolution = 1024;
		const float m_minSize = 0.5f;
	
		EditParameters m_editParameters;
	};
}