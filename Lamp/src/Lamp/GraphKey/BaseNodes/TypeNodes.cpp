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

	class Float4Node : public Node
	{
	public:
		Float4Node()
		{
			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<glm::vec4>("Value", PropertyType::Float4)
			};

			name = "Float4";
			outputAttributes = outputs;
		}

	public:
		static std::string GetFactoryName() { return "Float4"; }
		static Ref<Node> Create() { return CreateRef<Float4Node>(); }
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

	class BuildFloat3Node : public Node
	{
	public:
		BuildFloat3Node()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Build", PropertyType::Void, LP_BIND_NODE_FUNCTION(BuildFloat3Node::Build)),
				InputAttributeConfig<float>("X", PropertyType::Float),
				InputAttributeConfig<float>("Y", PropertyType::Float),
				InputAttributeConfig<float>("Z", PropertyType::Float)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<glm::vec3>("Output", PropertyType::Float3)
			};

			name = "BuildFloat3";
			inputAttributes = inputs;
			outputAttributes = outputs;
		}

		void Build()
		{
			glm::vec3 out = { GetInput<float>(1), GetInput<float>(2), GetInput<float>(3) };
			ActivateOutput(0, out);
		}

	public:
		static std::string GetFactoryName() { return "BuildFloat3"; }
		static Ref<Node> Create() { return CreateRef<BuildFloat3Node>(); }
	};

	class BuildFloat4Node : public Node
	{
	public:
		BuildFloat4Node()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Build", PropertyType::Void, LP_BIND_NODE_FUNCTION(BuildFloat4Node::Build)),
				InputAttributeConfig<float>("X", PropertyType::Float),
				InputAttributeConfig<float>("Y", PropertyType::Float),
				InputAttributeConfig<float>("Z", PropertyType::Float),
				InputAttributeConfig<float>("W", PropertyType::Float)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<glm::vec4>("Output", PropertyType::Float4)
			};

			name = "BuildFloat4";
			inputAttributes = inputs;
			outputAttributes = outputs;
		}

		void Build()
		{
			glm::vec4 out = { GetInput<float>(1), GetInput<float>(2), GetInput<float>(3), GetInput<float>(4) };
			ActivateOutput(0, out);
		}

	public:
		static std::string GetFactoryName() { return "BuildFloat4"; }
		static Ref<Node> Create() { return CreateRef<BuildFloat4Node>(); }
	};

	class DecomposeFloat2Node : public Node 
	{
	public:
		DecomposeFloat2Node()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Decompose", PropertyType::Void, LP_BIND_NODE_FUNCTION(DecomposeFloat2Node::Decompose)),
				InputAttributeConfig<glm::vec2>("Float2", PropertyType::Float2)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<float>("X", PropertyType::Float),
				OutputAttributeConfig<float>("Y", PropertyType::Float)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "DecomposeFloat2";
		}

		void Decompose()
		{
			glm::vec2 in = GetInput<glm::vec2>(1);
			ActivateOutput(0, in.x);
			ActivateOutput(1, in.y);
		}

	public:
		static std::string GetFactoryName() { return "DecomposeFloat2"; }
		static Ref<Node> Create() { return CreateRef<DecomposeFloat2Node>(); }
	};

	class DecomposeFloat3Node : public Node
	{
	public:
		DecomposeFloat3Node()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Decompose", PropertyType::Void, LP_BIND_NODE_FUNCTION(DecomposeFloat3Node::Decompose)),
				InputAttributeConfig<glm::vec3>("Float3", PropertyType::Float3)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<float>("X", PropertyType::Float),
				OutputAttributeConfig<float>("Y", PropertyType::Float),
				OutputAttributeConfig<float>("Z", PropertyType::Float)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "DecomposeFloat3";
		}

		void Decompose()
		{
			glm::vec3 in = GetInput<glm::vec3>(1);
			ActivateOutput(0, in.x);
			ActivateOutput(1, in.y);
			ActivateOutput(2, in.z);
		}

	public:
		static std::string GetFactoryName() { return "DecomposeFloat3"; }
		static Ref<Node> Create() { return CreateRef<DecomposeFloat3Node>(); }
	};

	class DecomposeFloat4Node : public Node
	{
	public:
		DecomposeFloat4Node()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Decompose", PropertyType::Void, LP_BIND_NODE_FUNCTION(DecomposeFloat4Node::Decompose)),
				InputAttributeConfig<glm::vec4>("Float4", PropertyType::Float4)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<float>("X", PropertyType::Float),
				OutputAttributeConfig<float>("Y", PropertyType::Float),
				OutputAttributeConfig<float>("Z", PropertyType::Float),
				OutputAttributeConfig<float>("W", PropertyType::Float)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "DecomposeFloat4";
		}

		void Decompose()
		{
			glm::vec4 in = GetInput<glm::vec4>(1);
			ActivateOutput(0, in.x);
			ActivateOutput(1, in.y);
			ActivateOutput(2, in.z);
			ActivateOutput(3, in.w);
		}

	public:
		static std::string GetFactoryName() { return "DecomposeFloat4"; }
		static Ref<Node> Create() { return CreateRef<DecomposeFloat4Node>(); }
	};

	LP_REGISTER_NODE(IntNode, "Types");
	LP_REGISTER_NODE(FloatNode, "Types");
	LP_REGISTER_NODE(Float2Node, "Types");
	LP_REGISTER_NODE(Float3Node, "Types");
	LP_REGISTER_NODE(Float4Node, "Types");

	LP_REGISTER_NODE(BuildFloat2Node, "Utility");
	LP_REGISTER_NODE(BuildFloat3Node, "Utility");
	LP_REGISTER_NODE(BuildFloat4Node, "Utility");

	LP_REGISTER_NODE(DecomposeFloat2Node, "Utility");
	LP_REGISTER_NODE(DecomposeFloat3Node, "Utility");
	LP_REGISTER_NODE(DecomposeFloat4Node, "Utility");
}