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
				ComponentProperty prop(PropertyType::Int, "Create", &m_Int);
				Attribute attr(prop, AttributeType::Input);
				attributes.push_back(attr);
			}

			{
				ComponentProperty prop(PropertyType::Float3, "Position", &m_Position);
				Attribute attr(prop, AttributeType::Input);
				attributes.push_back(attr);
			}

			{
				ComponentProperty prop(PropertyType::Int, "Created", &m_Int);
				Attribute attr(prop, AttributeType::Output);
				attributes.push_back(attr);
			}

			name = "CreateEntity";
		}

	public:
		static std::string GetFactoryName() { return "CreateEntity"; }
		static Ref<Node> Create() { return CreateRef<CreateEntity>(); }

	private:
		uint32_t m_Int = 0;
		glm::vec3 m_Position;
	};

	LP_REGISTER_NODE(CreateEntity);
}