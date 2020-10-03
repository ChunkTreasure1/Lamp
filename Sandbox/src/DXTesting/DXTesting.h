#pragma once

#include <Lamp.h>

namespace DXTesting
{
	class DXTesting : public Lamp::Layer
	{
	public: 
		DXTesting();

		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;
	};
}