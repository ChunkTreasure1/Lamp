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
			inputAttributes =
			{
				InputAttributeConfig_Void("Add", PropertyType::Void, LP_BIND_NODE_FUNCTION(AddNode::Add)),
				InputAttributeConfig<float>("A", PropertyType::Float),
				InputAttributeConfig<float>("B", PropertyType::Float)
			};

			outputAttributes =
			{
				OutputAttributeConfig_Void("Done", PropertyType::Void),
				OutputAttributeConfig<float>("Result", PropertyType::Float),
			};

			name = "Add";
		}

	private:
		void Add()
		{
			float val = GetInput<float>(1) + GetInput<float>(2);
			ActivateOutput(1, val);
			ActivateOutput(0);
		}

	public:
		static std::string GetFactoryName() { return "AddNode"; }
		static Ref<Node> Create() { return CreateRef<AddNode>(); }

	private:
	};

	class SubtractNode : public Node
	{
	public:
		SubtractNode()
		{
			inputAttributes =
			{
				InputAttributeConfig_Void("Subtract", PropertyType::Void, LP_BIND_NODE_FUNCTION(SubtractNode::Subtract)),
				InputAttributeConfig<float>("A", PropertyType::Float),
				InputAttributeConfig<float>("B", PropertyType::Float)
			};

			outputAttributes =
			{
				OutputAttributeConfig_Void("Done", PropertyType::Void),
				OutputAttributeConfig<float>("Result", PropertyType::Float)
			};

			name = "Subtract";
		}

	private:
		void Subtract()
		{
			float val = GetInput<float>(1) - GetInput<float>(2);
			ActivateOutput(1, val);
			ActivateOutput(0);
		}

	public:
		static std::string GetFactoryName() { return "SubtractNode"; }
		static Ref<Node> Create() { return CreateRef<SubtractNode>(); }

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
				OutputAttributeConfig_Void("Done", PropertyType::Void),
				OutputAttributeConfig<float>("Result", PropertyType::Float)
			};

			name = "Multiply";
		}

	private:
		void Multiply()
		{
			float val = GetInput<float>(1) * GetInput<float>(2);
			ActivateOutput(1, val);
			ActivateOutput(0);
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
				OutputAttributeConfig_Void("Done", PropertyType::Void),
				OutputAttributeConfig<float>("Result", PropertyType::Float)
			};

			name = "Divide";
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
			ActivateOutput(1, val);
			ActivateOutput(0);
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