#pragma once

#include <Lamp.h>

#include <glm/glm.hpp>

namespace Sandbox2D
{
	class Sandbox2D : public Lamp::Layer 
	{
	public:
		Sandbox2D();

		virtual void Update(Lamp::Timestep ts) override;
		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;

		virtual void OnItemClicked(Lamp::File& file) override;

	private:
		Lamp::OrthographicCameraController* m_CameraController;
		glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
	};
}