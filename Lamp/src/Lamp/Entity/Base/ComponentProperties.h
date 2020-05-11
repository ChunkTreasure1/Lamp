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
		Float4,
		Color
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
				case PropertyType::Color:  return std::make_any<glm::vec4*>(static_cast<glm::vec4*>(compProperty.Value));
			}
		}

		static std::any GetValue(ComponentProperty& compProperty, void* pData)
		{
			switch (compProperty.PropertyType)
			{
				case PropertyType::String: return std::make_any<std::string*>(static_cast<std::string*>(pData));
				case PropertyType::Bool:   return std::make_any<bool*>(static_cast<bool*>(pData));
				case PropertyType::Int:	   return std::make_any<int*>(static_cast<int*>(pData));
				case PropertyType::Float:  return std::make_any<float*>(static_cast<float*>(pData));
				case PropertyType::Float2: return std::make_any<glm::vec2*>(static_cast<glm::vec2*>(pData));
				case PropertyType::Float3: return std::make_any<glm::vec3*>(static_cast<glm::vec3*>(pData));
				case PropertyType::Float4: return std::make_any<glm::vec4*>(static_cast<glm::vec4*>(pData));
				case PropertyType::Color:  return std::make_any<glm::vec4*>(static_cast<glm::vec4*>(pData));
			}
		}

		static PropertyType GetTypeFromString(const char* string)
		{
			if (strcmp(string, "String") == 0)
			{
				return PropertyType::String;
			}
			else if (strcmp(string, "Bool") == 0)
			{
				return PropertyType::Bool;
			}
			else if (strcmp(string, "Int") == 0)
			{
				return PropertyType::Int;
			}
			else if (strcmp(string, "Float") == 0)
			{
				return PropertyType::Float;
			}
			else if (strcmp(string, "Float2") == 0)
			{
				return PropertyType::Float2;
			}
			else if (strcmp(string, "Float3") == 0)
			{
				return PropertyType::Float3;
			}
			else if (strcmp(string, "Float4") == 0)
			{
				return PropertyType::Float4;
			}
			else if (strcmp(string, "Color") == 0)
			{
				return PropertyType::Color;
			}

			return PropertyType::Int;
		}
		static std::string GetStringFromType(PropertyType type)
		{
			switch (type)
			{
			case PropertyType::String: return "String";
			case PropertyType::Bool:   return "Bool";
			case PropertyType::Int:	   return "Int";
			case PropertyType::Float:  return "Float";
			case PropertyType::Float2: return "Float2";
			case PropertyType::Float3: return "Float3";
			case PropertyType::Float4: return "Float4";
			case PropertyType::Color:  return "Color";
			}

			return "";
		}

	private:
		std::vector<ComponentProperty> m_Properties;
	};
}