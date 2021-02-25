#pragma once

#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Link.h"

#define LP_BIND_NODE_FUNCTION(x) std::bind(&x, this)

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
		std::string name;
		void* value;
	};

	struct Attribute
	{
		using Func = std::function<void()>;

		Attribute() {}

		Attribute(PropertyType t, const std::string& name, void* value, AttributeType type)
			: type(type)
		{
			property.name = name;
			property.type = t;
			property.value = value;
		}

		NodeProperty property;
		AttributeType type;
		uint32_t id;
		Ref<Link> pLink = nullptr;
		Func func = NULL;
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

		void ActivateOutput(Attribute& attr);

		Entity* pEntity;
		std::string name;
		uint32_t id;

		std::vector<Attribute> attributes;
		std::vector<Ref<Link>> links;
	};
}