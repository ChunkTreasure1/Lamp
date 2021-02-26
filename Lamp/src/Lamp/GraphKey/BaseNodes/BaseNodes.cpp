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
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig_Void("OnStart", PropertyType::Void)
			};

			outputAttributes = outputs;

			name = "Start";
		}

	public:
		static std::string GetFactoryName() { return "StartNode"; }
		static Ref<Node> Create() { return CreateRef<StartNode>(); }
	};

	LP_REGISTER_NODE(StartNode);
}