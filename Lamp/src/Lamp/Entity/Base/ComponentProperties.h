#pragma once

#include <vector>
#include <string>
#include <any>

#include <glm/glm.hpp>

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
				case PropertyType::String: return std::any_cast<std::string>(static_cast<std::string*>(compProperty.Value));
				case PropertyType::Bool:   return std::any_cast<bool>(static_cast<bool*>(compProperty.Value));
				case PropertyType::Int:	   return std::any_cast<int>(static_cast<int*>(compProperty.Value));
				case PropertyType::Float:  return std::any_cast<float>(static_cast<float*>(compProperty.Value));
				case PropertyType::Float2: return std::any_cast<glm::vec2>(static_cast<glm::vec2*>(compProperty.Value));
				case PropertyType::Float3: return std::any_cast<glm::vec3>(static_cast<glm::vec3*>(compProperty.Value));
				case PropertyType::Float4: return std::any_cast<glm::vec4>(static_cast<glm::vec4*>(compProperty.Value));
			}
		}

		static void DrawProperty(ComponentProperty& compProperty)
		{
			switch (compProperty.PropertyType)
			{
				case PropertyType::Float3:
				{
					glm::vec3 p = std::any_cast<glm::vec3>(GetValue(compProperty));

					float f[3] = { p.x, p.y, p.z };
					ImGui::InputFloat3(compProperty.Name.c_str(), f, 3);
					break;
				}
			}
		}

	private:
		std::vector<ComponentProperty> m_Properties;
	};
}