#include "lppch.h"
#include "Lamp/GraphKey/Node.h"
#include "Lamp/GraphKey/NodeRegistry.h"

#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/EntityManager.h"

namespace Lamp
{
	class CreateEntityNode : public Node
	{
	public:
		CreateEntityNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig_Void("Create", PropertyType::Void, LP_BIND_NODE_FUNCTION(CreateEntityNode::GetEntity)),
				InputAttributeConfig<glm::vec3>("Position", PropertyType::Float3),
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<int>("Id", PropertyType::Int),
				OutputAttributeConfig_Void("Created", PropertyType::Void)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "CreateEntity";
		}

	private:
		void GetEntity()
		{
			Entity* pEntity = Entity::Create();
			pEntity->SetPosition(GetInput<glm::vec3>(1));
			
			int i = (int)pEntity->GetID();
			outputAttributes[0].data = i;

			ActivateOutput<int>(0, i);
			ActivateOutput(1);
		}

	public:
		static std::string GetFactoryName() { return "CreateEntityNode"; }
		static Ref<Node> Create() { return CreateRef<CreateEntityNode>(); }

	private:
		uint32_t m_Id = -1;
	};

	class EntityNode : public Node
	{
	public:
		EntityNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig<int>("Id", PropertyType::Int)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig<int>("Id", PropertyType::Int)
			};

			inputAttributes = inputs;
			outputAttributes = outputs;

			name = "Entity";
		}

	public:
		static std::string GetFactoryName() { return "EntityNode"; }
		static Ref<Node> Create() { return CreateRef<EntityNode>(); }

	private:
		uint32_t m_Id = -1;
	};

	class AddComponentNode : public Node 
	{
	public:
		AddComponentNode()
		{
			std::vector<InputAttribute> inputs =
			{
				InputAttributeConfig<int>("EntityId", PropertyType::Int),
				InputAttributeConfig_Void("Add", PropertyType::Void, LP_BIND_NODE_FUNCTION(AddComponentNode::AddComponent)),
				InputAttributeConfig<std::vector<std::pair<std::string, bool>>>("Component", PropertyType::Selectable)
			};

			std::vector<OutputAttribute> outputs =
			{
				OutputAttributeConfig_Void("Added", PropertyType::Void)
			};

			name = "Add Component";

			auto& vec = std::any_cast<std::vector<std::pair<std::string, bool>>&>(inputs[2].data);
			for (auto& key : ComponentRegistry::s_Methods())
			{
				std::string val(key.first);
				vec.push_back(std::make_pair<>(val, false));
			}

			outputAttributes = outputs;
			inputAttributes = inputs;
		}

	private:
		void AddComponent()
		{
			Entity* pEnt = EntityManager::Get()->GetEntityFromId(GetInput<int>(0));
			if (!pEnt)
			{
				return;
			}

			auto& vec = GetInput<std::vector<std::pair<std::string, bool>>>(2);

			for (auto& s : vec)
			{
				if (s.second)
				{
					pEnt->AddComponent(ComponentRegistry::Create(s.first));

					Lamp::EntityComponentAddedEvent e;
					pEnt->OnEvent(e);

					ActivateOutput(0);
					return;
				}
			}
		}

	public:
		static std::string GetFactoryName() { return "AddComponentNode"; }
		static Ref<Node> Create() { return CreateRef<AddComponentNode>(); }
	};

	LP_REGISTER_NODE(CreateEntityNode);
	LP_REGISTER_NODE(EntityNode);
	LP_REGISTER_NODE(AddComponentNode);
}