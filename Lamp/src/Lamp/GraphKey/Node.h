#pragma once

#include "Lamp/Objects/Entity/Base/Entity.h"
#include "Link.h"

#define LP_BIND_NODE_FUNCTION(x) std::bind(&x, this)

namespace Lamp
{
	class Attribute
	{
	public:
		Attribute()
		{}

		virtual ~Attribute() {}

	public:
		std::string name;
		uint32_t id;
		PropertyType type;
		std::vector<Ref<Link>> pLinks;
		std::any data;
		bool linkable = true;
	};

	struct InputAttribute : public Attribute
	{
		using Func = std::function<void()>;

		InputAttribute(const std::string& n, bool linkable)
		{
			name = n;
			linkable = linkable;
		}

		std::map<std::string, int> enums;
		Func function = NULL;
	};

	struct OutputAttribute : public Attribute
	{
		OutputAttribute(const std::string& n, bool linkable)
		{
			name = n;
			linkable = linkable;
		}
	};

	struct Node
	{
		Node()
			: name("")
		{}

		Node(std::shared_ptr<Node>& node)
		{
			entityId = node->entityId;
			name = node->name;
			id = node->id;
			pLinks = node->pLinks;
			inputAttributes = node->inputAttributes;
			outputAttributes = node->outputAttributes;
		}

	public:
		virtual void OnEvent(Event& e) {}

	public:
		template<typename T>
		InputAttribute InputAttributeConfig(const std::string& name, PropertyType type, std::map<std::string, int> enums, bool linkable = true)
		{
			InputAttribute attr(name, linkable);
			attr.data = T();
			attr.enums = enums;
			attr.type = type;

			return attr;
		}
		
		template<typename T>
		InputAttribute InputAttributeConfig(const std::string& name, PropertyType type, bool linkable = true)
		{
			InputAttribute attr(name, linkable);
			attr.data = T();
			attr.type = type;

			return attr;
		}

		template<typename T>
		InputAttribute InputAttributeConfig(const std::string& name, PropertyType type, InputAttribute::Func f, bool linkable = true)
		{
			InputAttribute attr(name, linkable);
			attr.data = T();
			attr.type = type;
			attr.function = f;

			return attr;
		}

		InputAttribute InputAttributeConfig_Void(const std::string& name, PropertyType type, bool linkable = true)
		{
			InputAttribute attr(name, linkable);
			attr.type = type;

			return attr;
		}

		InputAttribute InputAttributeConfig_Void(const std::string& name, PropertyType type, InputAttribute::Func f, bool linkable = true)
		{
			InputAttribute attr(name, linkable);
			attr.type = type;
			attr.function = f;

			return attr;
		}

		template<typename T>
		OutputAttribute OutputAttributeConfig(const std::string& name, PropertyType type, bool linkable = true)
		{
			OutputAttribute attr(name, linkable);
			attr.type = type;
			attr.data = T();

			return attr;
		}

		OutputAttribute OutputAttributeConfig_Void(const std::string& name, PropertyType type, bool linkable = true)
		{
			OutputAttribute attr(name, linkable);
			attr.type = type;

			return attr;
		}

		template<typename T>
		void ActivateOutput(uint32_t index, T data)
		{
			if (outputAttributes[index].pLinks.size() > 0)
			{
				for (auto& link : outputAttributes[index].pLinks)
				{
					link->pInput->data = data;
					if (link->pInput->function != NULL)
					{
						link->pInput->function();
					}
				}
			}
		}

		void ActivateOutput(uint32_t index)
		{
			if (outputAttributes[index].pLinks.size() > 0)
			{
				for (auto& link : outputAttributes[index].pLinks)
				{
					if (link->pInput->function != NULL)
					{
						link->pInput->function();
					}
				}
			}
		}

		template<typename T>
		T& GetInput(uint32_t index)
		{
			return std::any_cast<T&>(inputAttributes[index].data);
		}

	public:
		int entityId = -1;
		std::string name;
		uint32_t id;
		glm::vec2 position;
		bool needsEntity = false;

		std::vector<Ref<Link>> pLinks;
		std::vector<InputAttribute> inputAttributes;
		std::vector<OutputAttribute> outputAttributes;
	};
}