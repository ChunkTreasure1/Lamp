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

	struct Attribute
	{
		Attribute() {}

		Attribute(ComponentProperty prop, AttributeType type)
			: property(prop), type(type)
		{}

		ComponentProperty property;
		AttributeType type;
		uint32_t id;
	};

	struct Node
	{
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
		std::vector<Link> links;
	};
}