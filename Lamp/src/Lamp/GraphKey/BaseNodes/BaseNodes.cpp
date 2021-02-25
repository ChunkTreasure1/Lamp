#include "lppch.h"
#include "Lamp/GraphKey/Node.h"

#include "Lamp/GraphKey/NodeRegistry.h"

namespace Lamp
{ 
	class StartNode : public Node
	{
	public:
		StartNode()
		{
			{
				Attribute attr(PropertyType::Void, "OnStart", nullptr, AttributeType::Output);
				attributes.push_back(attr);
			}

			name = "Start";
		}

	public:
		static std::string GetFactoryName() { return "Start"; }
		static Ref<Node> Create() { return CreateRef<StartNode>(); }
	};

	LP_REGISTER_NODE(StartNode);
}