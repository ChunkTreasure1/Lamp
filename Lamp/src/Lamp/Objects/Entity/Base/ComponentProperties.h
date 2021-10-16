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
		Path,
		Bool,
		Int,
		Float,
		Float2,
		Float3,
		Float4,
		Color3,
		Color4,
		Void,
		Selectable,
		EntityId
	};

	struct ComponentProperty
	{
		ComponentProperty(PropertyType propertyType, const std::string& name, void* value)
			: propertyType(propertyType), name(name), value(value)
		{}

		ComponentProperty() 
			: propertyType(PropertyType::Void), value(nullptr)
		{}

		std::string name;
		void* value;
		PropertyType propertyType;

		static uint32_t PropertyTypeToSize(Lamp::PropertyType type)
		{
			switch (type)
			{
			case Lamp::PropertyType::String: return 0;
			case Lamp::PropertyType::Path: return 0;
			case Lamp::PropertyType::Bool: return 1;
			case Lamp::PropertyType::Int: return 4;
			case Lamp::PropertyType::Float: return 4;
			case Lamp::PropertyType::Float2: return 2 * 4;
			case Lamp::PropertyType::Float3: return 3 * 4;
			case Lamp::PropertyType::Float4: return 3 * 4;
			case Lamp::PropertyType::Color3: return 3 * 4;
			case Lamp::PropertyType::Color4: return 4 * 4;
			case Lamp::PropertyType::Void: return 0;
			case Lamp::PropertyType::Selectable: return 0;
			case Lamp::PropertyType::EntityId: return 0;
			default:
				break;
			}
		}
	};

	class ComponentProperties
	{
	public:
		ComponentProperties(std::initializer_list<ComponentProperty> properties)
			: m_Properties(properties)
		{}

		ComponentProperties()
		{}

		inline std::vector<ComponentProperty>& GetProperties() { return m_Properties; }

	public:
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
			else if (strcmp(string, "Color3") == 0)
			{
				return PropertyType::Color3;
			}
			else if (strcmp(string, "Color4") == 0)
			{
				return PropertyType::Color4;
			}
			else if (strcmp(string, "Path") == 0)
			{
				return PropertyType::Path;
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
				case PropertyType::Color3: return "Color3";
				case PropertyType::Color4: return "Color4";
				case PropertyType::Path:   return "Path";
			}

			return "";
		}

	private:
		std::vector<ComponentProperty> m_Properties;
	};
}