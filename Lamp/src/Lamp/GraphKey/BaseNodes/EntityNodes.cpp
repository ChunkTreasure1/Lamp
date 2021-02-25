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
				attributes.push_back(attr);
			}

			{
				Attribute attr(PropertyType::Float3, "Position", &m_Position, AttributeType::Input);
				attributes.push_back(attr);
			}

			{
				Attribute attr(PropertyType::Float, "Created", &v, AttributeType::Output);
				attributes.push_back(attr);
			}

			name = "CreateEntity";
		}

	public:
		static std::string GetFactoryName() { return "CreateEntity"; }
		static Ref<Node> Create() { return CreateRef<CreateEntity>(); }

	private:
		uint32_t m_Int = 0;
		float v = 1.f;
		glm::vec3 m_Position;
	};

	LP_REGISTER_NODE(CreateEntity);
}