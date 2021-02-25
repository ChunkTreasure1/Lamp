#pragma once

#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Link.h"

namespace Lamp
{
	enum class AttributeType
	{
		Input,
		Output
	};
	
	struct NodeProperty
	{
		PropertyType type;
		std::string Name;
		void* Value;
	};

	struct Attribute
	{
		Attribute() {}

		Attribute(PropertyType t, const std::string& name, void* value, AttributeType type)
			: type(type)
		{
			property.Name = name;
			property.type = t;
			property.Value = value;
		}

		NodeProperty property;
		AttributeType type;
		uint32_t id;
		Ref<Link> pLink = nullptr;
	};

	struct Node
	{
		using Func = std::function<void()>;

		Node()
			: pEntity(nullptr), name("")
		{}

		Node(std::shared_ptr<Node>& node)
		{
			pEntity = node->pEntity;
			name = node->name;
			id = node->id;
			attributes = node->attributes;
			links = node->links;
		}

		Entity* pEntity;
		std::string name;
		uint32_t id;

		std::vector<Attribute> attributes;
		std::vector<Ref<Link>> links;
		std::vector<Func> inputFuncs;
		std::vector<Func> outputFuncs;
	};
}