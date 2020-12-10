#pragma once

#include "Sandbox3D.h"

namespace Sandbox3D
{
	class ModelImporter
	{
	public:
		ModelImporter();

		void Update();
		void UpdateCamera(Lamp::Timestep ts);
		void Render();
		
		inline bool& GetIsOpen() { return m_Open; }

	private:
		void RenderGrid();
		void UpdatePerspective();
		void UpdateProperties();

	private:
		bool m_Open = false;
		bool m_HoveringPerspective = false;
		glm::vec2 m_PerspectiveSize;

		Ref<Lamp::FrameBuffer> m_FrameBuffer;
		Ref<Lamp::Model> m_pModelToImport;
		Ref<Lamp::PerspectiveCameraController> m_Camera;
		Ref<Lamp::Shader> m_DefaultShader;
	};
}