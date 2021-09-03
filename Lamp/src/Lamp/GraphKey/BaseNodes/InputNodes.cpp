#include "lppch.h"
#include "Lamp/GraphKey/Node.h"
#include "Lamp/GraphKey/NodeRegistry.h"

#include "Lamp/Event/KeyEvent.h"
#include "Lamp/Event/MouseEvent.h"

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
	class KeyReleasedNode : public Node 
	{
	public:
		KeyReleasedNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig<int>("Keycode", PropertyType::Int)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig_Void("Released", PropertyType::Void)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "KeyReleased";
		}

		bool KeyReleased(KeyReleasedEvent& e)
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
			dispatcher.Dispatch<KeyReleasedEvent>(LP_BIND_EVENT_FN(KeyReleasedNode::KeyReleased));
		}

	public:
		static std::string GetFactoryName() { return "KeyReleased"; }
		static Ref<Node> Create() { return CreateRef<KeyReleasedNode>(); }
	};
	class MouseButtonPressedNode : public Node 
	{
	public:
		MouseButtonPressedNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig<int>("Keycode", PropertyType::Int)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig_Void("Pressed", PropertyType::Void)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "MouseButtonPressed";
		}

		bool ButtonPressed(MouseButtonPressedEvent& e)
		{
			if (e.GetMouseButton() == GetInput<int>(0))
			{
				ActivateOutput(0);
			}

			return false;
		}

		virtual void OnEvent(Event& e) override
		{
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<MouseButtonPressedEvent>(LP_BIND_EVENT_FN(MouseButtonPressedNode::ButtonPressed));
		}

	public:
		static std::string GetFactoryName() { return "MouseButtonPressed"; }
		static Ref<Node> Create() { return CreateRef<MouseButtonPressedNode>(); }
	};
	class MouseButtonReleasedNode : public Node 
	{
	public:
		MouseButtonReleasedNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig<int>("Keycode", PropertyType::Int)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig_Void("Released", PropertyType::Void)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "MouseButtonReleased";
		}

		bool ButtonReleased(MouseButtonReleasedEvent& e)
		{
			if (e.GetMouseButton() == GetInput<int>(0))
			{
				ActivateOutput(0);
			}

			return false;
		}

		virtual void OnEvent(Event& e) override
		{
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<MouseButtonReleasedEvent>(LP_BIND_EVENT_FN(MouseButtonReleasedNode::ButtonReleased));
		}

	public:
		static std::string GetFactoryName() { return "MouseButtonReleased"; }
		static Ref<Node> Create() { return CreateRef<MouseButtonReleasedNode>(); }
	};
	class MouseMovedNode : public Node
	{
	public:
		MouseMovedNode()
		{
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<float>("X", PropertyType::Float),
				OutputAttributeConfig<float>("Y", PropertyType::Float)
			};

			outputAttributes = outputs;
			name = "MouseMoved";
		}

		bool MouseMoved(MouseMovedEvent& e)
		{
			ActivateOutput(0, e.GetX());
			ActivateOutput(1, e.GetY());

			return false;
		}

		virtual void OnEvent(Event& e) override
		{
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(MouseMovedNode::MouseMoved));
		}

	public:
		static std::string GetFactoryName() { return "MouseMoved"; }
		static Ref<Node> Create() { return CreateRef<MouseMovedNode>(); }
	};

	LP_REGISTER_NODE(KeyPressedNode, "Input");
	LP_REGISTER_NODE(KeyReleasedNode, "Input");
	LP_REGISTER_NODE(MouseButtonPressedNode, "Input");
	LP_REGISTER_NODE(MouseButtonReleasedNode, "Input");
	LP_REGISTER_NODE(MouseMovedNode, "Input");
}