#pragma once

#include <string>

namespace Lamp
{
	class Layer
	{
	public:
		Layer(const std::string& name) {}
		virtual ~Layer() {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void Update() {}

		inline const std::string& GetName() const { return m_Name; }

	private:
		std::string m_Name;
	};
}