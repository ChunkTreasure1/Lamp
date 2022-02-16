#pragma once

#include "EditorWindow.h"

#include <Lamp/Event/KeyEvent.h>
#include <Lamp/Event/MouseEvent.h>
#include <Lamp/Event/ApplicationEvent.h>

namespace Lamp
{
	class SubMesh;
}

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
			Flatten,
			RaiseLower,
			Smooth,
		};
		
		struct EditParameters
		{
			bool isEditing = false;

			float innerRadius = 0.f;
			float outerRadius = 1.f;
			float hardness = 0.f;

			EditMode currentEditMode = EditMode::Flatten;
		};

		bool OnUpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);
		bool OnKeyPressedEvent(Lamp::KeyPressedEvent& e);
		bool OnRenderEvent(Lamp::AppRenderEvent& e);
		bool OnMouseScrolledEvent(Lamp::MouseScrolledEvent& e);

		std::vector<uint32_t> GenerateEmptyTextureData(uint32_t resolution);
		uint32_t CalculateTerrainSize();
		uint32_t GetHeightMapResolution();
		float GetMetersPerPixel();

		std::vector<const char*> m_perPixelSize = { "0.5 meter", "1 meter", "1.5 meters", "2 meters" };
		std::vector<const char*> m_resolutions = { "1024x1024", "2048x2048", "4096x4096" };
	
		int m_selectedSize = 0;
		int m_selectedResolution = 0;
		float m_scrollSpeedMultiplier = 0.5f;

		const uint32_t m_minResolution = 1024;
		const float m_minSize = 0.5f;
	
		EditParameters m_editParameters;

		Ref<Lamp::SubMesh> m_circleMesh;
	};
}