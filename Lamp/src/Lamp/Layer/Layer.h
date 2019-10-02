#pragma once
#include "Lamp/Event/Event.h"
#include "Lamp/Core/Timestep.h"

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
		virtual void Update(Timestep ts) {}

		virtual void OnImGuiRender(Timestep ts) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnItemClicked(File& file) {}

		inline const std::string& GetName() const { return m_DebugName; }
	private:
		std::string m_DebugName;
	};
}