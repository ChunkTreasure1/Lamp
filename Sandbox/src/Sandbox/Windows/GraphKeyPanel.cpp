#include "GraphKeyPanel.h"

#include <ImNodes.h>

#include <Lamp/Objects/Entity/Base/ComponentRegistry.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <imgui/imgui_stdlib.h>

#include <Lamp/GraphKey/Link.h>
#include <Lamp/GraphKey/NodeRegistry.h>
#include <Lamp/Objects/Entity/Base/Entity.h>

#include <Lamp/Utility/UIUtility.h>

namespace Sandbox
{
	using namespace Lamp;

	GraphKeyPanel::GraphKeyPanel(std::string_view name)
		: BaseWindow(name)
	{
		//CreateComponentNodes();
	}

	void GraphKeyPanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(GraphKeyPanel::UpdateImGui));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(GraphKeyPanel::OnUpdate));
	}

	void GraphKeyPanel::SetCurrentlyOpenGraph(Ref<Lamp::GraphKeyGraph> graph, uint32_t entity)
	{
		m_CurrentEntityId = entity;
		m_CurrentlyOpenGraph = graph;

		for (auto& n : graph->GetSpecification().nodes)
		{
			ImNodes::SetNodeEditorSpacePos(n->id, ImVec2{ n->position.x, n->position.y });
		}
	}

	bool GraphKeyPanel::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_IsOpen)
		{
			return false;
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(m_name.c_str(), &m_IsOpen);
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_name.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();

		UpdateNodeWindow();
		UpdateNodeList();
		UpdatePropertiesWindow();
		UpdateGraphList();

		return false;
	}

	bool GraphKeyPanel::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		if (Input::IsKeyPressed(LP_KEY_DELETE))
		{
			const int numSelectedNodes = ImNodes::NumSelectedNodes();
			if (numSelectedNodes > 0)
			{
				std::vector<int> selectedNodes;
				selectedNodes.resize(numSelectedNodes);

				ImNodes::GetSelectedNodes(selectedNodes.data());

				for (auto& node : selectedNodes)
				{
					RemoveNode(node);
				}
			}

			const int numSelectedLinks = ImNodes::NumSelectedLinks();
			if (numSelectedLinks > 0)
			{
				std::vector<int> selectedLinks;
				selectedLinks.resize(numSelectedLinks);

				ImNodes::GetSelectedLinks(selectedLinks.data());
				for (auto& link : selectedLinks)
				{
					RemoveLink(link);
				}
			}
		}

		return true;
	}

	void GraphKeyPanel::UpdateNodeWindow()
	{
		ImGui::Begin("Main", &m_IsOpen);
		if (m_CurrentlyOpenGraph)
		{
			ImGui::Text(m_CurrentlyOpenGraph->GetSpecification().name.c_str());
		}

		bool mainHovered = IsHovered({ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y }, { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y });
		ImNodes::BeginNodeEditor();
		if (m_CurrentlyOpenGraph)
		{
			for (auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
			{
				DrawNode(node);
			}
		}

		if (m_CurrentlyOpenGraph)
		{
			//Draw Links
			for (auto& link : m_CurrentlyOpenGraph->GetSpecification().links)
			{
				ImNodes::Link(link->id, link->pOutput->id, link->pInput->id);
			}
		}

		if (m_CurrentlyHovered < 0)
		{
			if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_NoOpenOverExistingPopup))
			{
				if (ImGui::MenuItem("test"))
				{
				}

				ImGui::EndPopup();
			}
		}

		ImNodes::EndNodeEditor();
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("NODE_ITEM"))
			{
				const char* node = (const char*)pPayload->Data;
				AddNode(std::string(node));
			}

			ImGui::EndDragDropTarget();
		}

		m_CurrentlyHovered = -1;
		ImNodes::IsNodeHovered(&m_CurrentlyHovered);

		int startAttr, endAttr;
		if (ImNodes::IsLinkCreated(&startAttr, &endAttr))
		{
			Node* pStartNode = nullptr;
			Node* pEndNode = nullptr;

			Attribute* pStartAttr = nullptr;
			Attribute* pEndAttr = nullptr;

			if (m_CurrentlyOpenGraph)
			{
				for (auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
				{
					for (uint32_t i = 0; i < node->inputAttributes.size(); i++)
					{
						if (node->inputAttributes[i].id == startAttr)
						{
							pStartNode = node.get();
							pStartAttr = &node->inputAttributes[i];

							break;
						}

						if (node->inputAttributes[i].id == endAttr)
						{
							pEndNode = node.get();
							pEndAttr = &node->inputAttributes[i];

							break;
						}
					}

					for (uint32_t i = 0; i < node->outputAttributes.size(); i++)
					{
						if (node->outputAttributes[i].id == endAttr)
						{
							pEndNode = node.get();
							pEndAttr = &node->outputAttributes[i];

							break;
						}

						if (node->outputAttributes[i].id == startAttr)
						{
							pStartNode = node.get();
							pStartAttr = &node->outputAttributes[i];

							break;
						}
					}

				}

				if (pStartAttr && pEndAttr)
				{
					if (pStartAttr->type == pEndAttr->type)
					{
						Ref<Link> pL = CreateRef<Link>();
						pL->id = m_CurrentlyOpenGraph->GetCurrentId()++;

						if (auto* p = dynamic_cast<InputAttribute*>(pStartAttr))
						{
							pL->pInput = p;
						}
						if (auto* p = dynamic_cast<OutputAttribute*>(pStartAttr))
						{
							pL->pOutput = p;
						}

						if (auto* p = dynamic_cast<InputAttribute*>(pEndAttr))
						{
							pL->pInput = p;
						}
						if (auto* p = dynamic_cast<OutputAttribute*>(pEndAttr))
						{
							pL->pOutput = p;
						}

						pStartNode->pLinks.push_back(pL);
						pEndNode->pLinks.push_back(pL);

						pStartAttr->pLinks.push_back(pL);
						pEndAttr->pLinks.push_back(pL);

						m_CurrentlyOpenGraph->GetSpecification().links.push_back(pL);
					}
				}
			}
		}

		const int numSelectedNodes = ImNodes::NumSelectedNodes();
		if (numSelectedNodes == 1)
		{
			int selNode = 0;
			ImNodes::GetSelectedNodes(&selNode);

			for (auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
			{
				if (node->id == selNode)
				{
					m_SelectedNode = node;
				}
			}
		}

		ImGui::End();
	}

	void GraphKeyPanel::UpdateNodeList()
	{
		if (!m_IsOpen)
		{
			return;
		}

		ImGui::Begin("Nodes");
		if (ImGui::TreeNode("Nodes"))
		{
			std::unordered_map<std::string, std::vector<std::pair<std::string, Lamp::NodeRegistry::TCreateMethod>>> sorted;

			for (auto& key : Lamp::NodeRegistry::s_Methods())
			{
				sorted[Lamp::NodeRegistry::GetCategory(key.first)].emplace_back(key);
			}

			int i = 0;
			for (auto& key : sorted)
			{
				if (ImGui::TreeNode(key.first.c_str()))
				{
					for (auto& p : key.second)
					{
						ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

						ImGui::TreeNodeEx((void*)i, nodeFlags, p.first.c_str());
						if (ImGui::BeginDragDropSource())
						{
							const char* node = p.first.c_str();
							ImGui::SetDragDropPayload("NODE_ITEM", node, strlen(node) * sizeof(char), ImGuiCond_Once);

							ImGui::EndDragDropSource();
						}

						if (ImGui::IsItemClicked() && m_CurrentlyOpenGraph)
						{

						}

						i++;
					}
					ImGui::TreePop();

				}
			}

			//for (auto& node : m_ComponentNodes)
			//{
			//	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			//	ImGui::TreeNodeEx((void*)i, nodeFlags, node->name.c_str());
			//	if (ImGui::IsItemClicked())
			//	{
			//		Ref<Node> n = CreateRef<Node>(node);
			//		n->id = s_Ids++;

			//		for (uint32_t i = 0; i < node->inputAttributes.size(); i++)
			//		{
			//			n->inputAttributes[i].id = s_Ids++;
			//		}


			//		for (int i = 0; i < node->outputAttributes.size(); i++)
			//		{
			//			n->outputAttributes[i].id = s_Ids++;
			//		}

			//		m_ExistingNodes.push_back(n);
			//	}

			//	i++;
			//}

			ImGui::TreePop();
		}

		ImGui::End();
	}

	void GraphKeyPanel::UpdatePropertiesWindow()
	{
		if (!m_IsOpen)
		{
			return;
		}

		ImGui::Begin("Properties##GraphKey");

		if (m_SelectedNode)
		{
			if (UI::TreeNodeFramed("Inputs"))
			{
				UI::PushId();
				UI::BeginProperties("inputProps", false);

				for (auto& input : m_SelectedNode->inputAttributes)
				{
					bool propertyChanged = false;
					switch (input.type)
					{
						case Lamp::PropertyType::Int: propertyChanged = UI::Property(input.name, std::any_cast<int&>(input.data)); break;
						case Lamp::PropertyType::Bool: propertyChanged = UI::Property(input.name, std::any_cast<bool&>(input.data)); break;
						case Lamp::PropertyType::Float: propertyChanged = UI::Property(input.name, std::any_cast<float&>(input.data)); break;
						case Lamp::PropertyType::Float2: propertyChanged = UI::Property(input.name, std::any_cast<glm::vec2&>(input.data)); break;
						case Lamp::PropertyType::Float3: propertyChanged = UI::Property(input.name, std::any_cast<glm::vec3&>(input.data)); break;
						case Lamp::PropertyType::Float4: propertyChanged = UI::Property(input.name, std::any_cast<glm::vec4&>(input.data)); break;
						case Lamp::PropertyType::String: propertyChanged = UI::Property(input.name, std::any_cast<std::string&>(input.data)); break;
						case Lamp::PropertyType::Path: propertyChanged = UI::Property(input.name, std::filesystem::path(std::any_cast<std::string&>(input.data))); break;
						case Lamp::PropertyType::Color3: propertyChanged = UI::Property(input.name, std::any_cast<glm::vec3&>(input.data), false); break;
						case Lamp::PropertyType::Color4: propertyChanged = UI::Property(input.name, std::any_cast<glm::vec4&>(input.data), true); break;
						case Lamp::PropertyType::Enum:
						{
							int currentValue = 0;
							for (const auto& item : input.enums)
							{
								if (std::any_cast<int&>(input.data) == item.second)
								{
									break;
								}

								currentValue++;
							}
							
							propertyChanged = UI::Property(input.name, currentValue, input.enums);
							
							if (propertyChanged)
							{
								uint32_t i = 0;
								for (const auto& item : input.enums)
								{
									if (i == currentValue)
									{
										std::any_cast<int&>(input.data) = item.second;
										break;
									}

									i++;
								}
							}

							break;
						}
					}
				}

				UI::EndProperties(false);
				UI::PopId();

				UI::TreeNodePop();
			}

			if (UI::TreeNodeFramed("Outputs"))
			{
				UI::BeginProperties("outputProps", false);

				for (auto& output : m_SelectedNode->outputAttributes)
				{
					bool propertyChanged = false;
					switch (output.type)
					{
						case Lamp::PropertyType::Int: propertyChanged = UI::Property(output.name, std::any_cast<int&>(output.data)); break;
						case Lamp::PropertyType::Bool: propertyChanged = UI::Property(output.name, std::any_cast<bool&>(output.data)); break;
						case Lamp::PropertyType::Float: propertyChanged = UI::Property(output.name, std::any_cast<float&>(output.data)); break;
						case Lamp::PropertyType::Float2: propertyChanged = UI::Property(output.name, std::any_cast<glm::vec2&>(output.data)); break;
						case Lamp::PropertyType::Float3: propertyChanged = UI::Property(output.name, std::any_cast<glm::vec3&>(output.data)); break;
						case Lamp::PropertyType::Float4: propertyChanged = UI::Property(output.name, std::any_cast<glm::vec4&>(output.data)); break;
						case Lamp::PropertyType::String: propertyChanged = UI::Property(output.name, std::any_cast<std::string&>(output.data)); break;
						case Lamp::PropertyType::Path: propertyChanged = UI::Property(output.name, std::filesystem::path(std::any_cast<std::string&>(output.data))); break;
						case Lamp::PropertyType::Color3: propertyChanged = UI::Property(output.name, std::any_cast<glm::vec3&>(output.data), false); break;
						case Lamp::PropertyType::Color4: propertyChanged = UI::Property(output.name, std::any_cast<glm::vec4&>(output.data), true); break;

						default:
							break;
					}
				}

				UI::EndProperties(false);
				UI::TreeNodePop();
			}
		}

		ImGui::End();
	}

	void GraphKeyPanel::UpdateGraphList()
	{
		if (!m_IsOpen)
		{
			return;
		}

		ImGui::Begin("Graphs");

		for (auto& pEnt : g_pEnv->pLevel->GetEntities())
		{
			if (pEnt.second)
			{
				if (pEnt.second->GetGraphKeyGraph())
				{
					if (ImGui::Selectable(pEnt.second->GetName().c_str()))
					{
						SetCurrentlyOpenGraph(pEnt.second->GetGraphKeyGraph(), pEnt.second->GetID());
					}
				}
			}
		}

		ImGui::End();
	}

	void GraphKeyPanel::CreateComponentNodes()
	{
		for (const auto& pC : Lamp::ComponentRegistry::s_Methods())
		{
			m_BaseComponents.push_back(pC.second());
		}

		for (const auto& pComp : m_BaseComponents)
		{
			Ref<Node> node = CreateRef<Node>();
			node->name = pComp->GetName();
			node->needsEntity = true;

			std::vector<InputAttribute> inputs;
			inputs.push_back(node->InputAttributeConfig<int>("EntityId", PropertyType::Int));
			//inputs[0].data = std::make_any<int>(&node->entityId);

			for (auto& prop : pComp->GetComponentProperties().GetProperties())
			{
				switch (prop.propertyType)
				{
					case Lamp::PropertyType::Bool: inputs.push_back(node->InputAttributeConfig<bool>(prop.name, Lamp::PropertyType::Bool)); break;
					case Lamp::PropertyType::Int: inputs.push_back(node->InputAttributeConfig<int>(prop.name, Lamp::PropertyType::Int)); break;
					case Lamp::PropertyType::Float: inputs.push_back(node->InputAttributeConfig<float>(prop.name, Lamp::PropertyType::Float)); break;
					case Lamp::PropertyType::Float2: inputs.push_back(node->InputAttributeConfig<glm::vec2>(prop.name, Lamp::PropertyType::Float2)); break;
					case Lamp::PropertyType::Float3: inputs.push_back(node->InputAttributeConfig<glm::vec3>(prop.name, Lamp::PropertyType::Float3)); break;
					case Lamp::PropertyType::Float4: inputs.push_back(node->InputAttributeConfig<glm::vec4>(prop.name, Lamp::PropertyType::Float4)); break;
					case Lamp::PropertyType::Color3: inputs.push_back(node->InputAttributeConfig<glm::vec3>(prop.name, Lamp::PropertyType::Float3)); break;
					case Lamp::PropertyType::Color4: inputs.push_back(node->InputAttributeConfig<glm::vec4>(prop.name, Lamp::PropertyType::Float4)); break;
					case Lamp::PropertyType::Path: inputs.push_back(node->InputAttributeConfig<std::string>(prop.name, Lamp::PropertyType::Path)); break;
					case Lamp::PropertyType::String: inputs.push_back(node->InputAttributeConfig<std::string>(prop.name, Lamp::PropertyType::String)); break;
					case Lamp::PropertyType::Void: inputs.push_back(node->InputAttributeConfig_Void(prop.name, Lamp::PropertyType::Bool)); break;
				}
			}

			m_ComponentNodes.push_back(node);
		}
	}

	void GraphKeyPanel::RemoveNode(uint32_t id)
	{
		m_CurrentlyOpenGraph->RemoveNode(id);
	}

	void GraphKeyPanel::RemoveLink(uint32_t id)
	{
		m_CurrentlyOpenGraph->RemoveLink(id);
	}

	void GraphKeyPanel::DrawNode(Ref<Lamp::Node> node)
	{
		ImNodes::BeginNode(node->id);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(node->id);
		if (pos.x != node->position.x || pos.y != node->position.y)
		{
			node->position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text(node->name.c_str());
		ImNodes::EndNodeTitleBar();

		Entity* pEntity = Entity::Get(node->entityId);

		if (!pEntity && node->needsEntity)
		{
			ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "No entity assigned!");
		}

		ImVec2 cursorPos = ImGui::GetCursorPos();

		if (!node->inputAttributes.empty())
		{
			cursorPos.x += 120.f;
		}

		for (int i = 0; i < node->inputAttributes.size(); i++)
		{
			InputAttribute& attr = node->inputAttributes[i];

			auto pinShape = attr.pLinks.empty() ? ImNodesPinShape_Triangle : ImNodesPinShape_TriangleFilled;
			ImNodes::BeginInputAttribute(attr.id, pinShape);
			{
				DrawInput(attr, node);

			}
			ImNodes::EndInputAttribute();
		}

		ImGui::SetCursorPos(cursorPos);

		for (int i = 0; i < node->outputAttributes.size(); i++)
		{
			OutputAttribute& attr = node->outputAttributes[i];

			auto pinShape = attr.pLinks.empty() ? ImNodesPinShape_Triangle : ImNodesPinShape_TriangleFilled;
			ImNodes::BeginOutputAttribute(attr.id, pinShape);
			{
				DrawOutput(attr, node);
			}
			ImNodes::EndOutputAttribute();

			cursorPos.y += 20.f;
			ImGui::SetCursorPos(cursorPos);
		}

		ImNodes::EndNode();
	}

	void GraphKeyPanel::DrawInput(Lamp::InputAttribute& attr, Ref<Lamp::Node> node, bool isProperties)
	{
		auto& prop = attr;

		Entity* pEntity = Entity::Get(node->id);
		float width = ImNodes::GetNodeDimensions(node->id).x;

		switch (prop.type)
		{
			case Lamp::PropertyType::Int:
			{
				int& p = std::any_cast<int&>(prop.data);
				ImGui::Text("%s = %i", prop.name.c_str(), p);
				break;
			}

			case Lamp::PropertyType::Bool:
			{
				bool& p = std::any_cast<bool&>(prop.data);
				std::string text = p ? "true" : "false";
				ImGui::Text("%s = %s", prop.name.c_str(), text.c_str());
				break;
			}

			case Lamp::PropertyType::Float:
			{
				float& f = std::any_cast<float&>(prop.data);
				ImGui::Text("%s = %.2f", prop.name.c_str(), f);
				break;
			}

			case Lamp::PropertyType::Float2:
			{
				glm::vec2& p = std::any_cast<glm::vec2&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f }", prop.name.c_str(), p.x, p.y);
				break;
			}

			case Lamp::PropertyType::Float3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z);
				break;
			}

			case Lamp::PropertyType::Float4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z, p.w);
				break;
			}

			case Lamp::PropertyType::String:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);
				ImGui::Text("%s = %s", prop.name.c_str(), s.c_str());
				break;
			}

			case Lamp::PropertyType::Path:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);
				ImGui::Text("%s = %s", prop.name.c_str(), s.c_str());
				break;
			}

			case Lamp::PropertyType::Color3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z);
				break;
			}

			case Lamp::PropertyType::Color4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z, p.w);
				break;
			}

			case Lamp::PropertyType::Void:
			{
				if (isProperties)
				{
					break;
				}

				ImGui::Text(prop.name.c_str());

				break;
			}

			case Lamp::PropertyType::Enum:
			{
				int index = std::any_cast<int&>(prop.data);
				std::string text;
				for (const auto& val : prop.enums)
				{
					if (val.second == index)
					{
						text = val.first;
						break;
					}
				}

				ImGui::Text("Value = %s", text.c_str());

				break;
			}

			case Lamp::PropertyType::Selectable:
			{
				auto& vec = std::any_cast<std::vector<std::pair<std::string, bool>>&>(prop.data);
				static std::string currentItem;

				ImGui::PushItemWidth(300.f);

				if (ImGui::BeginCombo(prop.name.c_str(), currentItem.c_str()))
				{
					for (auto& item : vec)
					{
						item.second = (currentItem == item.first);
						if (ImGui::Selectable(item.first.c_str(), item.second))
						{
							currentItem = item.first;
						}
						if (item.second)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				ImGui::PopItemWidth();
				break;
			}

			case Lamp::PropertyType::EntityId:
			{
				auto id = std::any_cast<int>(prop.data);
				node->entityId = id;
				pEntity = Entity::Get(node->id);
				if (pEntity)
				{
					ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), pEntity->GetName().c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Assign an entity!");
				}

				break;
			}
		}
	}

	void GraphKeyPanel::DrawOutput(Lamp::OutputAttribute& attr, Ref<Lamp::Node> node, bool isProperties)
	{
		auto& prop = attr;

		Entity* pEntity = Entity::Get(node->entityId);

		for (auto& link : attr.pLinks)
		{
			link->pInput->data = attr.data;
		}

		switch (prop.type)
		{
			case Lamp::PropertyType::Int:
			{
				int& p = std::any_cast<int&>(prop.data);
				ImGui::Text("%s = %i", prop.name.c_str(), p);
				break;
			}

			case Lamp::PropertyType::Bool:
			{
				bool& p = std::any_cast<bool&>(prop.data);
				std::string text = p ? "true" : "false";
				ImGui::Text("%s = %s", prop.name.c_str(), text.c_str());
				break;
			}

			case Lamp::PropertyType::Float:
			{
				float& f = std::any_cast<float&>(prop.data);
				ImGui::Text("%s = %.2f", prop.name.c_str(), f);
				break;
			}

			case Lamp::PropertyType::Float2:
			{
				glm::vec2& p = std::any_cast<glm::vec2&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f }", prop.name.c_str(), p.x, p.y);
				break;
			}

			case Lamp::PropertyType::Float3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z);
				break;
			}

			case Lamp::PropertyType::Float4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z, p.w);
				break;
			}

			case Lamp::PropertyType::String:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);
				ImGui::Text("%s = %s", prop.name.c_str(), s.c_str());
				break;
			}

			case Lamp::PropertyType::Path:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);
				ImGui::Text("%s = %s", prop.name.c_str(), s.c_str());
				break;
			}

			case Lamp::PropertyType::Color3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z);
				break;
			}

			case Lamp::PropertyType::Color4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);
				ImGui::Text("%s = { %.2f, %.2f, %.2f, %.2f }", prop.name.c_str(), p.x, p.y, p.z, p.w);
				break;
			}

			case Lamp::PropertyType::Enum:
			{


				break;
			}

			case Lamp::PropertyType::Void:
			{
				if (isProperties)
				{
					break;
				}
				ImGui::Text(prop.name.c_str());

				break;
			}
		}
	}

	bool GraphKeyPanel::IsHovered(const glm::vec2& pos, const glm::vec2& size)
	{
		ImGuiIO& io = ImGui::GetIO();
		glm::vec2 mousePos = { io.MousePos.x, io.MousePos.y };

		if (mousePos.x > pos.x
			&& mousePos.x < pos.x + size.x
			&& mousePos.y > pos.y
			&& mousePos.y < pos.y + size.y)
		{
			return true;
		}

		return false;
	}

	void GraphKeyPanel::AddNode(const std::string& name)
	{
		if (!m_CurrentlyOpenGraph)
		{
			return;
		}

		Ref<Node> n = NodeRegistry::s_Methods()[name]();
		n->id = m_CurrentlyOpenGraph->GetCurrentId()++;

		for (auto& inputAttribute : n->inputAttributes)
		{
			inputAttribute.id = m_CurrentlyOpenGraph->GetCurrentId()++;
		}

		for (auto& outputAttribute : n->outputAttributes)
		{
			outputAttribute.id = m_CurrentlyOpenGraph->GetCurrentId()++;
		}

		if (m_CurrentlyOpenGraph)
		{
			m_CurrentlyOpenGraph->GetSpecification().nodes.push_back(n);
		}
	}
}