#pragma once

#include <vector>
#include <string>
#include <any>

#include <glm/glm.hpp>
#include <imgui.h>

namespace Lamp
{
	enum class PropertyType
	{
		String,
		Bool,
		Int,
		Float,
		Float2,
		Float3,
		Float4
	};

	struct ComponentProperty
	{
		ComponentProperty(PropertyType propertyType, const std::string& name, void* value)
			: PropertyType(propertyType), Name(name), Value(value)
		{}

		std::string Name;
		void* Value;
		PropertyType PropertyType;
	};

	class ComponentProperties
	{
	public:
		ComponentProperties(std::initializer_list<ComponentProperty> properties)
			: m_Properties(properties)
		{}

		inline std::vector<ComponentProperty>& GetProperties() { return m_Properties; }

	public:
		static std::any GetValue(ComponentProperty& compProperty)
		{
			switch (compProperty.PropertyType)
			{
				case PropertyType::String: return std::make_any<std::string*>(static_cast<std::string*>(compProperty.Value));
				case PropertyType::Bool:   return std::make_any<bool*>(static_cast<bool*>(compProperty.Value));
				case PropertyType::Int:	   return std::make_any<int*>(static_cast<int*>(compProperty.Value));
				case PropertyType::Float:  return std::make_any<float*>(static_cast<float*>(compProperty.Value));
				case PropertyType::Float2: return std::make_any<glm::vec2*>(static_cast<glm::vec2*>(compProperty.Value));
				case PropertyType::Float3: return std::make_any<glm::vec3*>(static_cast<glm::vec3*>(compProperty.Value));
				case PropertyType::Float4: return std::make_any<glm::vec4*>(static_cast<glm::vec4*>(compProperty.Value));
			}
		}

	private:
		std::vector<ComponentProperty> m_Properties;
	};
}