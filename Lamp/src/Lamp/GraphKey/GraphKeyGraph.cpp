#include "lppch.h"
#include "GraphKeyGraph.h"

#include "NodeRegistry.h"

namespace Lamp
{
	static const char* PropertyTypeToString(PropertyType type)
	{
		switch (type)
		{
			case Lamp::PropertyType::String:      return "string";
			case Lamp::PropertyType::Path:        return "path";
			case Lamp::PropertyType::Bool:        return "bool";
			case Lamp::PropertyType::Int:         return "int";
			case Lamp::PropertyType::Float:       return "float";
			case Lamp::PropertyType::Float2:      return "float2";
			case Lamp::PropertyType::Float3:      return "float3";
			case Lamp::PropertyType::Float4:      return "float4";
			case Lamp::PropertyType::Color3:      return "color3";
			case Lamp::PropertyType::Color4:      return "color4";
			case Lamp::PropertyType::Void:        return "void";
			case Lamp::PropertyType::Selectable:  return "selectable";
			case Lamp::PropertyType::EntityId:    return "entityid";
		}
	}

	static std::string ToString(const bool& var)
	{
		std::string str;
		if (var)
		{
			str = "1";
		}
		else
		{
			str = "0";
		}

		return str;
	}
	static std::string ToString(const int& var)
	{
		return std::to_string(var);
	}
	static std::string ToString(const float& var)
	{
		return std::to_string(var);
	}
	static std::string ToString(const glm::vec2& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y));
		return str;
	}
	static std::string ToString(const glm::vec3& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y) + "," + std::to_string(var.z));
		return str;
	}
	static std::string ToString(const glm::vec4& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y) + "," + std::to_string(var.z) + "," + std::to_string(var.w));
		return str;
	}
	static std::string ToString(uint32_t i)
	{
		return std::to_string(i);
	}

	static bool GetValue(char* val, bool& var)
	{
		if (val)
		{
			var = atoi(val) != 0;
			return true;
		}

		return false;
	}
	static bool GetValue(char* val, int& var)
	{
		if (val)
		{
			var = atoi(val);
			return true;
		}

		return false;
	}
	static bool GetValue(char* val, uint32_t& var)
	{
		if (val)
		{
			var = atoi(val);
			return true;
		}

		return false;
	}
	static bool GetValue(char* val, float& var)
	{
		if (val)
		{
			var = (float)atof(val);
			return true;
		}
		return false;
	}
	static bool GetValue(char* val, glm::vec2& var)
	{
		if (val)
		{
			float x, y;
			if (sscanf(val, "%f,%f", &x, &y) == 2)
			{
				var = glm::vec2(x, y);
				return true;
			}
		}
		return false;
	}
	static bool GetValue(char* val, glm::vec3& var)
	{
		if (val)
		{
			float x, y, z;
			if (sscanf(val, "%f,%f,%f", &x, &y, &z) == 3)
			{
				var = glm::vec3(x, y, z);
				return true;
			}
		}
		return false;
	}
	static bool GetValue(char* val, glm::vec4& var)
	{
		if (val)
		{
			float x, y, z, w;
			if (sscanf(val, "%f,%f,%f,%f", &x, &y, &z, &w) == 4)
			{
				var = glm::vec4(x, y, z, w);
				return true;
			}
		}
		return false;
	}

	GraphKeyGraph::GraphKeyGraph()
	{
	}

	GraphKeyGraph::GraphKeyGraph(const GraphKeyGraphSpecification& spec)
		: m_Specification(spec)
	{
	}

	void GraphKeyGraph::RemoveNode(uint32_t id)
	{
		auto func = [&id, this](Ref<Node>& node)
		{
			if (id == node->id)
			{
				for (auto& attr : node->inputAttributes)
				{
					if (attr.pLinks.size() > 0)
					{
						for (auto& link : attr.pLinks)
						{
							RemoveLink(link->id);
						}
					}
				}

				for (auto& attr : node->outputAttributes)
				{
					if (attr.pLinks.size() > 0)
					{
						for (auto& link : attr.pLinks)
						{
							RemoveLink(link->id);
						}
					}
				}

				return true;
			}

			return false;
		};
		m_Specification.nodes.erase(std::remove_if(m_Specification.nodes.begin(), m_Specification.nodes.end(), func), m_Specification.nodes.end());
	}

	void GraphKeyGraph::RemoveLink(uint32_t id)
	{
		for (auto& node : m_Specification.nodes)
		{
			for (auto& attr : node->inputAttributes)
			{
				if (attr.pLinks.size() == 0)
				{
					continue;
				}

				for (auto& link : attr.pLinks)
				{
					if (link->id == id)
					{
						if (auto it = std::find(attr.pLinks.begin(), attr.pLinks.end(), link); it != attr.pLinks.end())
						{
							attr.pLinks.erase(it);
						}
						break;
					}
				}
			}

			for (auto& attr : node->outputAttributes)
			{
				if (!attr.pLinks.size() == 0)
				{
					continue;
				}

				for (auto& link : attr.pLinks)
				{
					if (link->id == id)
					{
						if (auto it = std::find(attr.pLinks.begin(), attr.pLinks.end(), link); it != attr.pLinks.end())
						{
							attr.pLinks.erase(it);
						}
						break;
					}
				}
			}
		}

		auto func = [&id](Ref<Link>& link)
		{
			return id == link->id;
		};

		m_Specification.links.erase(std::remove_if(m_Specification.links.begin(), m_Specification.links.end(), func), m_Specification.links.end());
	}

	void GraphKeyGraph::OnEvent(Event& e)
	{
		for (auto& node : m_Specification.nodes)
		{
			node->OnEvent(e);
		}
	}

	void GraphKeyGraph::Save(Ref<GraphKeyGraph>& graph, rapidxml::xml_node<>* pRoot, rapidxml::xml_document<>& doc)
	{
		using namespace rapidxml;

		xml_node<>* pGraph = doc.allocate_node(node_element, "Graph");
		pGraph->append_attribute(doc.allocate_attribute("name", graph->GetSpecification().name.c_str()));

		xml_node<>* pNodes = doc.allocate_node(node_element, "Nodes");
		for (auto& n : graph->GetSpecification().nodes)
		{
			xml_node<>* pNode = doc.allocate_node(node_element, "Node");

			pNode->append_attribute(doc.allocate_attribute("name", n->name.c_str()));

			char* pId = doc.allocate_string(ToString(n->id).c_str());
			pNode->append_attribute(doc.allocate_attribute("id", pId));

			char* pPos = doc.allocate_string(ToString(n->position).c_str());
			pNode->append_attribute(doc.allocate_attribute("position", pPos));

			for (auto& i : n->inputAttributes)
			{
				AppendAttribute(i, "input", pNode, doc);
			}

			for (auto& i : n->outputAttributes)
			{
				AppendAttribute(i, "output", pNode, doc);
			}

			pNodes->append_node(pNode);
		}
		pGraph->append_node(pNodes);

		xml_node<>* pLinks = doc.allocate_node(node_element, "Links");
		for (auto& l : graph->GetSpecification().links)
		{
			xml_node<>* pLink = doc.allocate_node(node_element, "Link");

			char* pId = doc.allocate_string(ToString(l->id).c_str());
			pLink->append_attribute(doc.allocate_attribute("id", pId));

			if (l->pOutput)
			{
				char* pFrom = doc.allocate_string(ToString(l->pOutput->id).c_str());
				pLink->append_attribute(doc.allocate_attribute("from", pFrom));
			}
			if (l->pInput)
			{
				char* pTo = doc.allocate_string(ToString(l->pInput->id).c_str());
				pLink->append_attribute(doc.allocate_attribute("to", pTo));
			}

			pLinks->append_node(pLink);
		}
		pGraph->append_node(pLinks);

		pRoot->append_node(pGraph);
	}

	void GraphKeyGraph::Load(Ref<GraphKeyGraph>& graph, rapidxml::xml_node<>* pRoot)
	{
		using namespace rapidxml;
		graph->m_Specification.name = pRoot->first_attribute("name")->value();

		if (xml_node<>* pNodes = pRoot->first_node("Nodes"))
		{
			for (xml_node<>* pNode = pNodes->first_node("Node"); pNode; pNode = pNode->next_sibling())
			{
				Ref<Node> node = NodeRegistry::Create(pNode->first_attribute("name")->value());
				GetValue(pNode->first_attribute("id")->value(), node->id);
				if (node->id >= graph->GetCurrentId())
				{
					graph->SetCurrentId(node->id + 1);
				}

				glm::vec2 pos;
				GetValue(pNode->first_attribute("position")->value(), pos);
				node->position = pos;

				for (xml_node<>* pAttr = pNode->first_node("Attribute"); pAttr; pAttr = pAttr->next_sibling())
				{
					std::string attrType = pAttr->first_attribute("attrType")->value();
					if (attrType == "input")
					{
						std::string name = pAttr->first_attribute("name")->value();
						for (auto& attr : node->inputAttributes)
						{
							//This causes issues with attributes having the same name
							if (name == attr.name)
							{
								GetValue(pAttr->first_attribute("id")->value(), attr.id);
								if (attr.id >= graph->GetCurrentId())
								{
									graph->SetCurrentId(attr.id + 1);
								}
								LoadAttribute(attr, pAttr, name);

							}
						}
					}
					else if (attrType == "output")
					{
						std::string name = pAttr->first_attribute("name")->value();
						for (auto& attr : node->outputAttributes)
						{
							//This causes issues with attributes having the same name
							if (name == attr.name)
							{
								GetValue(pAttr->first_attribute("id")->value(), attr.id);
								if (attr.id >= graph->GetCurrentId())
								{
									graph->SetCurrentId(attr.id + 1);
								}
								LoadAttribute(attr, pAttr, name);
							}
						}
					}
				}

				graph->m_Specification.nodes.push_back(node);
			}
		}

		if (xml_node<>* pLinks = pRoot->first_node("Links"))
		{
			for (xml_node<>* pLink = pLinks->first_node("Link"); pLink; pLink = pLink->next_sibling())
			{
				uint32_t id;
				uint32_t from;
				uint32_t to;

				GetValue(pLink->first_attribute("id")->value(), id);
				GetValue(pLink->first_attribute("from")->value(), from);
				GetValue(pLink->first_attribute("to")->value(), to);

				if (id >= graph->GetCurrentId())
				{
					graph->SetCurrentId(id + 1);
				}

				Ref<Link> link = CreateRef<Link>();
				link->id = id;

				for (auto& n : graph->m_Specification.nodes)
				{
					for (auto& attr : n->outputAttributes)
					{
						if (from == attr.id)
						{
							link->pOutput = &attr;
							link->pOutput->pLinks.push_back(link);
							break;
						}
					}

					for (auto& attr : n->inputAttributes)
					{
						if (to == attr.id)
						{
							link->pInput = &attr;
							link->pInput->pLinks.push_back(link);
							break;
						}
					}

					if (link->pInput && link->pOutput)
					{
						link->pInput->data = link->pOutput->data;
					}
				}

				graph->m_Specification.links.push_back(link);
			}
		}
	}

	void GraphKeyGraph::AppendAttribute(Attribute& attr, const std::string& type, rapidxml::xml_node<>* pRoot, rapidxml::xml_document<>& doc)
	{
		using namespace rapidxml;

		xml_node<>* pInput = doc.allocate_node(node_element, "Attribute");

		char* pName = doc.allocate_string(attr.name.c_str());
		char* pAttrType = doc.allocate_string(type.c_str());
		char* pId = doc.allocate_string(ToString(attr.id).c_str());
		pInput->append_attribute(doc.allocate_attribute("id", pId));
		pInput->append_attribute(doc.allocate_attribute("attrType", pAttrType));
		pInput->append_attribute(doc.allocate_attribute("name", pName));
		pInput->append_attribute(doc.allocate_attribute("type", PropertyTypeToString(attr.type)));

		switch (attr.type)
		{
			case Lamp::PropertyType::String:
			{
				std::string data = std::any_cast<std::string>(attr.data);
				char* pString = doc.allocate_string(data.c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pString));
				break;
			}
			case Lamp::PropertyType::Path:
			{
				std::string data = std::any_cast<std::string>(attr.data);
				char* pPath = doc.allocate_string(data.c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pPath));
				break;
			}
			case Lamp::PropertyType::Bool:
			{
				bool data = std::any_cast<bool>(attr.data);
				char* pBool = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pBool));
				break;
			}
			case Lamp::PropertyType::Int:
			{
				int data = std::any_cast<int>(attr.data);
				char* pInt = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pInt));
				break;
			}
			case Lamp::PropertyType::Float:
			{
				float data = std::any_cast<float>(attr.data);
				char* pFloat = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pFloat));
				break;
			}
			case Lamp::PropertyType::Float2:
			{
				glm::vec2 data = std::any_cast<glm::vec2>(attr.data);
				char* pFloat2 = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pFloat2));
				break;
			}
			case Lamp::PropertyType::Float3:
			{
				glm::vec3 data = std::any_cast<glm::vec3>(attr.data);
				char* pFloat3 = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pFloat3));
				break;
			}
			case Lamp::PropertyType::Float4:
			{
				glm::vec4 data = std::any_cast<glm::vec4>(attr.data);
				char* pFloat4 = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pFloat4));
				break;
			}
			case Lamp::PropertyType::Color3:
			{
				glm::vec3 data = std::any_cast<glm::vec3>(attr.data);
				char* pFloat3 = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pFloat3));
				break;
			}
			case Lamp::PropertyType::Color4:
			{
				glm::vec4 data = std::any_cast<glm::vec4>(attr.data);
				char* pFloat4 = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pFloat4));
				break;
			}
			case Lamp::PropertyType::Void:
			{
				pInput->append_attribute(doc.allocate_attribute("data", ""));
				break;
			}
			case Lamp::PropertyType::Selectable:
			{
				pInput->append_attribute(doc.allocate_attribute("data", ""));
				break;
			}
			case Lamp::PropertyType::EntityId:
			{
				int data = std::any_cast<int>(attr.data);
				char* pEntityId = doc.allocate_string(ToString(data).c_str());
				pInput->append_attribute(doc.allocate_attribute("data", pEntityId));
				break;
			}
		}

		pRoot->append_node(pInput);
	}
	void GraphKeyGraph::LoadAttribute(Attribute& attr, rapidxml::xml_node<>* pRoot, const std::string& name)
	{
		switch (attr.type)
		{
			case Lamp::PropertyType::String:
			{
				attr.data = std::any(std::string(pRoot->first_attribute("data")->value()));
				break;
			}
			case Lamp::PropertyType::Path:
			{
				attr.data = std::any(std::string(pRoot->first_attribute("data")->value()));
				break;
			}
			case Lamp::PropertyType::Bool:
			{
				bool val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);

				break;
			}
			case Lamp::PropertyType::Int:
			{
				int val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);
				break;
			}
			case Lamp::PropertyType::Float:
			{
				float val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);
				break;
			}
			case Lamp::PropertyType::Float2:
			{
				glm::vec2 val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);
				break;
			}
			case Lamp::PropertyType::Float3:
			{
				glm::vec3 val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);
				break;
			}
			case Lamp::PropertyType::Float4:
			{
				glm::vec4 val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);
				break;
			}
			case Lamp::PropertyType::Color3:
			{
				glm::vec3 val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);
				break;
			}
			case Lamp::PropertyType::Color4:
			{
				glm::vec4 val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);
				break;
			}
			case Lamp::PropertyType::Void:
			{
				break;
			}
			case Lamp::PropertyType::Selectable:
			{
				break;
			}
			case Lamp::PropertyType::EntityId:
			{
				int val;
				GetValue(pRoot->first_attribute("data")->value(), val);
				attr.data = std::any(val);

				break;
			}
		}
	}
}