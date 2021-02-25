#include "lppch.h"
#include "Lamp/GraphKey/Node.h"
#include "Lamp/GraphKey/NodeRegistry.h"

namespace Lamp
{
	class CreateEntity : public Node
	{
	public:
		CreateEntity()
		{
			{
				Attribute attr(PropertyType::Void, "Create", nullptr, AttributeType::Input);
				attr.func = LP_BIND_NODE_FUNCTION(CreateEntity::Test);
				attributes.push_back(attr);
			}

			{
				Attribute attr(PropertyType::Float3, "Position", &m_Position, AttributeType::Input);
				attributes.push_back(attr);
			}

			{
				Attribute attr(PropertyType::Void, "Created", nullptr, AttributeType::Output);
				attributes.push_back(attr);
			}

			name = "CreateEntity";
		}

		void Test()
		{
			ActivateOutput(attributes[2]);
		}

	public:
		static std::string GetFactoryName() { return "CreateEntity"; }
		static Ref<Node> Create() { return CreateRef<CreateEntity>(); }

	private:
		glm::vec3 m_Position{0.f};
	};

	LP_REGISTER_NODE(CreateEntity);
}