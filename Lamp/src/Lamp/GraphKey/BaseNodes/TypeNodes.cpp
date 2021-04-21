#include "lppch.h"
#include "Lamp/GraphKey/Node.h"
#include "Lamp/GraphKey/NodeRegistry.h"

namespace Lamp
{
	class IntNode : public Node
	{
	public:
		IntNode()
		{
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<int>("Value", PropertyType::Int)
			};

			name = "Int";
			outputAttributes = outputs;
		}

	public:
		static std::string GetFactoryName() { return "Int"; }
		static Ref<Node> Create() { return CreateRef<IntNode>(); }
	};

	class FloatNode : public Node
	{
	public:
		FloatNode()
		{
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<float>("Value", PropertyType::Float)
			};

			name = "Float";
			outputAttributes = outputs;
		}

	public:
		static std::string GetFactoryName() { return "Float"; }
		static Ref<Node> Create() { return CreateRef<FloatNode>(); }
	};

	class Float2Node : public Node
	{
	public:
		Float2Node()
		{
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<glm::vec2>("Value", PropertyType::Float2)
			};

			name = "Float2";
			outputAttributes = outputs;
		}

	public:
		static std::string GetFactoryName() { return "Float2"; }
		static Ref<Node> Create() { return CreateRef<Float2Node>(); }
	};

	class Float3Node : public Node
	{
	public:
		Float3Node()
		{
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<glm::vec3>("Value", PropertyType::Float3)
			};

			name = "Float3";
			outputAttributes = outputs;
		}

	public:
		static std::string GetFactoryName() { return "Float3"; }
		static Ref<Node> Create() { return CreateRef<Float3Node>(); }
	};

	class BuildFloat2Node : public Node 
	{
	public:
		BuildFloat2Node()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Build", PropertyType::Void, LP_BIND_NODE_FUNCTION(BuildFloat2Node::Build)),
				InputAttributeConfig<float>("X", PropertyType::Float),
				InputAttributeConfig<float>("Y", PropertyType::Float)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<glm::vec2>("Output", PropertyType::Float2)
			};

			name = "BuildFloat2";
			inputAttributes = inputs;
			outputAttributes = outputs;
		}

		void Build()
		{
			glm::vec2 out = { GetInput<float>(1), GetInput<float>(2) };
			ActivateOutput(0, out);
		}

	public:
		static std::string GetFactoryName() { return "BuildFloat2"; }
		static Ref<Node> Create() { return CreateRef<BuildFloat2Node>(); }
	};

	LP_REGISTER_NODE(IntNode, "Types");
	LP_REGISTER_NODE(FloatNode, "Types");
	LP_REGISTER_NODE(Float2Node, "Types");
	LP_REGISTER_NODE(Float3Node, "Types");
	LP_REGISTER_NODE(BuildFloat2Node, "Utility");
}