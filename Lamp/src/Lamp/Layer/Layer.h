#pragma once
#include "Lamp/Event/Event.h"
#include "Lamp/Core/Time/Timestep.h"

#include "Lamp/Input/FileSystem.h"

namespace Lamp
{
	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;
		
		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnImGuiRender(Timestep ts) {}
		virtual void OnRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	private:
		std::string m_DebugName;
	};
}