#include "lppch.h"
#include "Lamp/GraphKey/Node.h"
#include "Lamp/GraphKey/NodeRegistry.h"

#include "Lamp/Event/KeyEvent.h"

namespace Lamp
{
	class KeyPressedNode : public Node
	{
	public:
		KeyPressedNode()
		{
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig_Void("Pressed", PropertyType::Void)
			};

			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig<int>("Key", PropertyType::Int)
			};

			outputAttributes = outputs;
			inputAttributes = inputs;

			name = "KeyPressed";
		}

		bool KeyPressed(KeyPressedEvent& e)
		{
			if (e.GetKeyCode() == GetInput<int>(0))
			{
				ActivateOutput(0);
			}

			return false;
		}

		virtual void OnEvent(Event& e) override 
		{
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(KeyPressedNode::KeyPressed));
		}

	public:
		static std::string GetFactoryName() { return "KeyPressed"; }
		static Ref<Node> Create() { return CreateRef<KeyPressedNode>(); }
	};

	LP_REGISTER_NODE(KeyPressedNode);
}