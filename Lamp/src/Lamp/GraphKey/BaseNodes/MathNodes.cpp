#include "lppch.h"

#include "Lamp/GraphKey/Node.h"
#include "Lamp/GraphKey/NodeRegistry.h"

namespace Lamp
{
	class AddNode : public Node 
	{
	public:
		AddNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Add", PropertyType::Void, LP_BIND_NODE_FUNCTION(AddNode::Add)),
				InputAttributeConfig<float>("A", PropertyType::Float),
				InputAttributeConfig<float>("B", PropertyType::Float)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<float>("Result", PropertyType::Float),
			};

			name = "Add";
			inputAttributes = inputs;
		}

	private:
		void Add()
		{
			float val = GetInput<float>(1) + GetInput<float>(2);
			ActivateOutput(0, val);
		}

	public:
		static std::string GetFactoryName() { return "AddNode"; }
		static Ref<Node> Create() { return CreateRef<Node>(); }

	private:
	};

	class SubtractNode : public Node
	{
	public:
		SubtractNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Add", PropertyType::Void, LP_BIND_NODE_FUNCTION(SubtractNode::Subtract)),
				InputAttributeConfig<float>("A", PropertyType::Float),
				InputAttributeConfig<float>("B", PropertyType::Float)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<float>("Result", PropertyType::Float),
			};

			name = "Subtract";
			inputAttributes = inputs;
		}

	private:
		void Subtract()
		{
			float val = GetInput<float>(1) - GetInput<float>(2);
			ActivateOutput(0, val);
		}

	public:
		static std::string GetFactoryName() { return "SubtractNode"; }
		static Ref<Node> Create() { return CreateRef<Node>(); }

	private:
	};

	class MultiplyNode : public Node 
	{
	public:
		MultiplyNode()
		{
			inputAttributes =
			{
				InputAttributeConfig_Void("Multiply", PropertyType::Void, LP_BIND_NODE_FUNCTION(MultiplyNode::Multiply)),
				InputAttributeConfig<float>("A", PropertyType::Float),
				InputAttributeConfig<float>("B", PropertyType::Float),
			};

			outputAttributes =
			{
				OutputAttributeConfig<float>("Result", PropertyType::Float)
			};

			name = "Multiply";
		}

	private:
		void Multiply()
		{
			float val = GetInput<float>(1) * GetInput<float>(2);
			ActivateOutput(0, val);
		}

	public:
		static std::string GetFactoryName() { return "MultiplyNode"; }
		static Ref<Node> Create() { return CreateRef<MultiplyNode>(); }
	};

	class DivideNode : public Node 
	{
	public:
		DivideNode()
		{
			inputAttributes =
			{
				InputAttributeConfig_Void("Divide", PropertyType::Void, LP_BIND_NODE_FUNCTION(DivideNode::Divide)),
				InputAttributeConfig<float>("A", PropertyType::Float),
				InputAttributeConfig<float>("B", PropertyType::Float)
			};

			outputAttributes =
			{
				OutputAttributeConfig<float>("Result", PropertyType::Float)
			};
		}

	private:
		void Divide()
		{
			if (GetInput<float>(1) == 0.f || GetInput<float>(2) == 0.f)
			{
				ActivateOutput(0, 0.f);
				return;
			}

			float val = GetInput<float>(1) / GetInput<float>(2);
			ActivateOutput(0, val);
		}

	public:
		static std::string GetFactoryName() { return "DivideNode"; }
		static Ref<Node> Create() { return CreateRef<DivideNode>(); }
	};

	LP_REGISTER_NODE(AddNode);
	LP_REGISTER_NODE(SubtractNode);
	LP_REGISTER_NODE(MultiplyNode);
	LP_REGISTER_NODE(DivideNode);
}