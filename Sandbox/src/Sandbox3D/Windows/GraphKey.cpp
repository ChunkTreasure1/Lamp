#include "GraphKey.h"

#include <imnodes.h>

#include <Lamp/Objects/Entity/Base/ComponentRegistry.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <imgui/imgui_stdlib.h>

#include <Lamp/GraphKey/Link.h>
#include <Lamp/GraphKey/NodeRegistry.h>
#include <Lamp/Objects/Entity/Base/Entity.h>

namespace Sandbox3D
{
	using namespace Lamp;

	GraphKey::GraphKey(std::string_view name)
		: BaseWindow(name)
	{
		//CreateComponentNodes();
	}

	void GraphKey::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(GraphKey::UpdateImGui));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(GraphKey::OnUpdate));
	}

	void GraphKey::SetCurrentlyOpenGraph(Ref<Lamp::GraphKeyGraph>& graph)
	{
		m_CurrentlyOpenGraph = graph;

		for (auto& n : graph->GetSpecification().nodes)
		{
			imnodes::SetNodeEditorSpacePos(n->id, ImVec2{ n->position.x, n->position.y });
		}
	}

	bool GraphKey::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_IsOpen)
		{
			return false;
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(m_Name.c_str(), &m_IsOpen);
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_Name.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();

		UpdateNodeWindow();
		UpdateNodeList();
		UpdatePropertiesWindow();
		UpdateGraphList();

		return false;
	}

	bool GraphKey::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		if (Input::IsKeyPressed(LP_KEY_DELETE))
		{
			const int numSelectedNodes = imnodes::NumSelectedNodes();
			if (numSelectedNodes > 0)
			{
				std::vector<int> selectedNodes;
				selectedNodes.resize(numSelectedNodes);

				imnodes::GetSelectedNodes(selectedNodes.data());

				for (auto& node : selectedNodes)
				{
					RemoveNode(node);
				}
			}

			const int numSelectedLinks = imnodes::NumSelectedLinks();
			if (numSelectedLinks > 0)
			{
				std::vector<int> selectedLinks;
				selectedLinks.resize(numSelectedLinks);

				imnodes::GetSelectedLinks(selectedLinks.data());
				for (auto& link : selectedLinks)
				{
					RemoveLink(link);
				}
			}
		}

		return true;
	}

	void GraphKey::UpdateNodeWindow()
	{
		ImGui::Begin("Main", &m_IsOpen);
		if (m_CurrentlyOpenGraph)
		{
			ImGui::Text(m_CurrentlyOpenGraph->GetSpecification().name.c_str());
		}

		bool mainHovered = IsHovered({ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y }, { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y });
		imnodes::BeginNodeEditor();
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
				imnodes::Link(link->id, link->pOutput->id, link->pInput->id);
			}
		}

		imnodes::EndNodeEditor();

		int startAttr, endAttr;
		if (imnodes::IsLinkCreated(&startAttr, &endAttr))
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

						pStartAttr->pLink = pL;
						pEndAttr->pLink = pL;

						m_CurrentlyOpenGraph->GetSpecification().links.push_back(pL);
					}
				}
			}

			const int numSelectedNodes = imnodes::NumSelectedNodes();
			if (numSelectedNodes == 1)
			{
				int selNode = 0;
				imnodes::GetSelectedNodes(&selNode);

				for (auto& node : m_CurrentlyOpenGraph->GetSpecification().nodes)
				{
					if (node->id == selNode)
					{
						m_SelectedNode = node;
					}
				}
			}
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && mainHovered)
		{
			imnodes::IsNodeHovered(&m_CurrentlyHovered);
			ImGui::OpenPopup("RightClick");
		}

		UpdateRightClickPopup();

		ImGui::End();
	}

	void GraphKey::UpdateNodeList()
	{
		if (!m_IsOpen)
		{
			return;
		}

		ImGui::Begin("Nodes");
		if (ImGui::TreeNode("Nodes"))
		{
			int i = 0;
			for (auto& key : Lamp::NodeRegistry::s_Methods())
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

				ImGui::TreeNodeEx((void*)i, nodeFlags, key.first.c_str());
				if (ImGui::IsItemClicked())
				{
					Ref<Node> n = key.second();
					n->id = m_CurrentlyOpenGraph->GetCurrentId()++;

					for (uint32_t i = 0; i < n->inputAttributes.size(); i++)
					{
						n->inputAttributes[i].id = m_CurrentlyOpenGraph->GetCurrentId()++;
					}

					for (int i = 0; i < n->outputAttributes.size(); i++)
					{
						n->outputAttributes[i].id = m_CurrentlyOpenGraph->GetCurrentId()++;
					}

					if (m_CurrentlyOpenGraph)
					{
						m_CurrentlyOpenGraph->GetSpecification().nodes.push_back(n);
					}
				}

				i++;
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

	void GraphKey::UpdatePropertiesWindow()
	{
		if (!m_IsOpen)
		{
			return;
		}

		ImGui::Begin("Properties##GraphKey");

		if (m_SelectedNode)
		{
			for (auto& input : m_SelectedNode->inputAttributes)
			{
				DrawInput(input, m_SelectedNode);
			}
			for (auto& output : m_SelectedNode->outputAttributes)
			{
				DrawOutput(output, m_SelectedNode);
			}
		}

		ImGui::End();
	}

	void GraphKey::UpdateRightClickPopup()
	{
		if (ImGui::BeginPopup("RightClick"))
		{
			if (m_CurrentlyHovered > -1)
			{
				if (ImGui::Selectable("Assign graph entity"))
				{

				}

				if (ImGui::Selectable("Remove"))
				{
					RemoveNode(m_CurrentlyHovered);
				}
			}

			if (ImGui::Selectable("Test"))
			{

			}

			ImGui::EndPopup();
		}
	}

	void GraphKey::UpdateGraphList()
	{
		if (!m_IsOpen)
		{
			return;
		}

		ImGui::Begin("Graphs");

		for (auto& pEnt : g_pEnv->pEntityManager->GetEntities())
		{
			if (pEnt.second)
			{
				if (pEnt.second->GetGraphKeyGraph())
				{
					if (ImGui::Selectable(pEnt.second->GetName().c_str()))
					{
						SetCurrentlyOpenGraph(pEnt.second->GetGraphKeyGraph());
					}
				}
			}
		}

		ImGui::End();
	}

	void GraphKey::CreateComponentNodes()
	{
		for (auto pC : Lamp::ComponentRegistry::s_Methods())
		{
			m_BaseComponents.push_back(pC.second());
		}

		for (auto pComp : m_BaseComponents)
		{
			Ref<Node> node = CreateRef<Node>();
			node->name = pComp->GetName();
			node->needsEntity = true;

			std::vector<InputAttribute> inputs;
			inputs.push_back(node->InputAttributeConfig<int>("EntityId", PropertyType::Int));
			//inputs[0].data = std::make_any<int>(&node->entityId);

			for (auto& prop : pComp->GetComponentProperties().GetProperties())
			{
				switch (prop.PropertyType)
				{
					case Lamp::PropertyType::Bool: inputs.push_back(node->InputAttributeConfig<bool>(prop.Name.c_str(), Lamp::PropertyType::Bool)); break;
					case Lamp::PropertyType::Int: inputs.push_back(node->InputAttributeConfig<int>(prop.Name.c_str(), Lamp::PropertyType::Int)); break;
					case Lamp::PropertyType::Float: inputs.push_back(node->InputAttributeConfig<float>(prop.Name.c_str(), Lamp::PropertyType::Float)); break;
					case Lamp::PropertyType::Float2: inputs.push_back(node->InputAttributeConfig<glm::vec2>(prop.Name.c_str(), Lamp::PropertyType::Float2)); break;
					case Lamp::PropertyType::Float3: inputs.push_back(node->InputAttributeConfig<glm::vec3>(prop.Name.c_str(), Lamp::PropertyType::Float3)); break;
					case Lamp::PropertyType::Float4: inputs.push_back(node->InputAttributeConfig<glm::vec4>(prop.Name.c_str(), Lamp::PropertyType::Float4)); break;
					case Lamp::PropertyType::Color3: inputs.push_back(node->InputAttributeConfig<glm::vec3>(prop.Name.c_str(), Lamp::PropertyType::Float3)); break;
					case Lamp::PropertyType::Color4: inputs.push_back(node->InputAttributeConfig<glm::vec4>(prop.Name.c_str(), Lamp::PropertyType::Float4)); break;
					case Lamp::PropertyType::Path: inputs.push_back(node->InputAttributeConfig<std::string>(prop.Name.c_str(), Lamp::PropertyType::Path)); break;
					case Lamp::PropertyType::String: inputs.push_back(node->InputAttributeConfig<std::string>(prop.Name.c_str(), Lamp::PropertyType::String)); break;
					case Lamp::PropertyType::Void: inputs.push_back(node->InputAttributeConfig_Void(prop.Name.c_str(), Lamp::PropertyType::Bool)); break;
				}
			}

			m_ComponentNodes.push_back(node);
		}
	}

	void GraphKey::RemoveNode(uint32_t id)
	{
		m_CurrentlyOpenGraph->RemoveNode(id);
	}

	void GraphKey::RemoveLink(uint32_t id)
	{
		m_CurrentlyOpenGraph->RemoveLink(id);
	}

	void GraphKey::DrawNode(Ref<Lamp::Node>& node)
	{
		imnodes::BeginNode(node->id);

		ImVec2 pos = imnodes::GetNodeEditorSpacePos(node->id);
		if (pos.x != node->position.x || pos.y != node->position.y)
		{
			node->position = { pos.x, pos.y };
		}

		imnodes::BeginNodeTitleBar();
		ImGui::Text(node->name.c_str());
		imnodes::EndNodeTitleBar();

		Entity* pEntity = g_pEnv->pEntityManager->GetEntityFromId(node->entityId);

		if (!pEntity && node->needsEntity)
		{
			ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "No entity assigned!");
		}

		for (int i = 0; i < node->inputAttributes.size(); i++)
		{
			InputAttribute& attr = node->inputAttributes[i];

			float nodeWidth = 100.f;
			if (attr.type == Lamp::PropertyType::Float3 || attr.type == Lamp::PropertyType::Float4
				|| attr.type == Lamp::PropertyType::Color3 || attr.type == Lamp::PropertyType::Color4)
			{
				nodeWidth = 200.f;
			}

			imnodes::BeginInputAttribute(attr.id);
			{
				float labelWidth = ImGui::CalcTextSize(attr.name.c_str()).x;
				ImGui::PushItemWidth(nodeWidth - labelWidth);

				DrawInput(attr, node);

				ImGui::PopItemWidth();
			}
			imnodes::EndInputAttribute();
		}


		for (int i = 0; i < node->outputAttributes.size(); i++)
		{
			OutputAttribute& attr = node->outputAttributes[i];

			float nodeWidth = 100.f;
			if (attr.type == Lamp::PropertyType::Float3 || attr.type == Lamp::PropertyType::Float4
				|| attr.type == Lamp::PropertyType::Color3 || attr.type == Lamp::PropertyType::Color4)
			{
				nodeWidth = 200.f;
			}

			imnodes::BeginOutputAttribute(attr.id);
			{
				float labelWidth = ImGui::CalcTextSize(attr.name.c_str()).x;
				ImGui::PushItemWidth(nodeWidth - labelWidth);

				DrawOutput(attr, node);

				ImGui::PopItemWidth();
			}
			imnodes::EndOutputAttribute();
		}


		imnodes::EndNode();
	}

	void GraphKey::DrawInput(Lamp::InputAttribute& attr, Ref<Lamp::Node>& node, bool isProperties)
	{
		auto& prop = attr;

		Entity* pEntity = g_pEnv->pEntityManager->GetEntityFromId(node->entityId);

		switch (prop.type)
		{
			case Lamp::PropertyType::Int:
			{
				int& p = std::any_cast<int&>(prop.data);

				if (!attr.pLink)
				{
					int v = p;

					ImGui::DragInt(prop.name.c_str(), &v);
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Bool:
			{
				bool& p = std::any_cast<bool&>(prop.data);

				if (!attr.pLink)
				{
					bool v = p;

					ImGui::Checkbox(prop.name.c_str(), &v);
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Float:
			{
				float& p = std::any_cast<float&>(prop.data);

				if (!attr.pLink)
				{
					float v = p;

					ImGui::DragFloat(prop.name.c_str(), &v);
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Float2:
			{
				glm::vec2& p = std::any_cast<glm::vec2&>(prop.data);

				if (attr.pLink)
				{
					glm::vec2 v = p;
					ImGui::DragFloat2(prop.name.c_str(), glm::value_ptr(v));
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Float3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);

				if (!attr.pLink)
				{
					glm::vec3 v = p;

					ImGui::DragFloat3(prop.name.c_str(), glm::value_ptr(v));
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ", " + std::to_string(p.z) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Float4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);

				if (!attr.pLink)
				{
					glm::vec4 v = p;

					ImGui::DragFloat4(prop.name.c_str(), glm::value_ptr(v));
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ", " + std::to_string(p.z) + ", " + std::to_string(p.w) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::String:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);

				if (!attr.pLink)
				{
					std::string v = s;

					ImGui::InputText(prop.name.c_str(), &v);
					if (v != s)
					{
						s = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + s + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Path:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);

				if (!attr.pLink)
				{
					std::string v = s;

					ImGui::InputText(prop.name.c_str(), &v);
					ImGui::SameLine();
					if (ImGui::Button("Open..."))
					{
						std::string path = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
						if (!path.empty())
						{
							v = path;
						}
					}

					if (v != s)
					{
						s = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + s + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Color3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);

				if (!attr.pLink)
				{
					glm::vec3 v = p;

					ImGui::ColorEdit3(prop.name.c_str(), glm::value_ptr(v));
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ", " + std::to_string(p.z) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Color4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);

				if (!attr.pLink)
				{
					glm::vec4 v = p;

					ImGui::ColorEdit4(prop.name.c_str(), glm::value_ptr(v));
					if (v != p)
					{
						p = v;

						if (pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ", " + std::to_string(p.z) + ", " + std::to_string(p.w) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

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

			case Lamp::PropertyType::Selectable:
			{
				auto& vec = std::any_cast<std::vector<std::pair<std::string, bool>>&>(prop.data);
				static std::string currentItem;

				ImGui::PushItemWidth(300.f);

				if (ImGui::BeginCombo(prop.name.c_str(), currentItem.c_str()))
				{
					for (int i = 0; i < vec.size(); i++)
					{
						vec[i].second = (currentItem == vec[i].first);
						if (ImGui::Selectable(vec[i].first.c_str(), vec[i].second))
						{
							currentItem = vec[i].first;
						}
						if (vec[i].second)
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
				pEntity = g_pEnv->pEntityManager->GetEntityFromId(id);
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

	void GraphKey::DrawOutput(Lamp::OutputAttribute& attr, Ref<Lamp::Node>& node, bool isProperties)
	{
		auto& prop = attr;

		Entity* pEntity = g_pEnv->pEntityManager->GetEntityFromId(node->entityId);

		switch (prop.type)
		{
			case Lamp::PropertyType::Int:
			{
				int& p = std::any_cast<int&>(prop.data);
				int v = p;

				ImGui::DragInt(prop.name.c_str(), &v);
				if (v != p)
				{
					p = v;

					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}

				break;
			}

			case Lamp::PropertyType::Bool:
			{
				bool& p = std::any_cast<bool&>(prop.data);
				bool v = p;

				ImGui::Checkbox(prop.name.c_str(), &v);
				if (v != p)
				{
					p = v;

					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}

				break;
			}

			case Lamp::PropertyType::Float:
			{
				float& p = std::any_cast<float&>(prop.data);
				float v = p;

				ImGui::DragFloat(prop.name.c_str(), &v);
				if (v != p)
				{
					p = v;

					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::Float2:
			{
				glm::vec2& p = std::any_cast<glm::vec2&>(prop.data);
				glm::vec2 v = p;

				ImGui::DragFloat2(prop.name.c_str(), glm::value_ptr(v));
				if (v != p)
				{
					p = v;

					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::Float3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);
				glm::vec3 v = p;

				ImGui::DragFloat3(prop.name.c_str(), glm::value_ptr(v));
				if (v != p)
				{
					p = v;

					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::Float4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);
				glm::vec4 v = p;

				ImGui::DragFloat4(prop.name.c_str(), glm::value_ptr(v));
				if (v != p)
				{
					p = v;
					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::String:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);
				std::string v = s;

				ImGui::InputText(prop.name.c_str(), &v);
				if (v != s)
				{
					s = v;
					if (attr.pLink)
					{
						attr.pLink->pInput->data = s;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::Path:
			{
				std::string& s = std::any_cast<std::string&>(prop.data);
				std::string v = s;

				ImGui::InputText(prop.name.c_str(), &v);
				ImGui::SameLine();
				if (ImGui::Button("Open..."))
				{
					std::string path = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
					if (!path.empty())
					{
						v = path;
					}
				}

				if (v != s)
				{
					s = v;
					if (attr.pLink)
					{
						attr.pLink->pInput->data = s;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::Color3:
			{
				glm::vec3& p = std::any_cast<glm::vec3&>(prop.data);
				glm::vec3 v = p;

				ImGui::ColorEdit3(prop.name.c_str(), glm::value_ptr(v));
				if (v != p)
				{
					p = v;
					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::Color4:
			{
				glm::vec4& p = std::any_cast<glm::vec4&>(prop.data);
				glm::vec4 v = p;

				ImGui::ColorEdit4(prop.name.c_str(), glm::value_ptr(v));
				if (v != p)
				{
					p = v;
					if (attr.pLink)
					{
						attr.pLink->pInput->data = p;
					}

					if (pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						pEntity->OnEvent(e);
					}
				}
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

	bool GraphKey::IsHovered(const glm::vec2& pos, const glm::vec2& size)
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
}