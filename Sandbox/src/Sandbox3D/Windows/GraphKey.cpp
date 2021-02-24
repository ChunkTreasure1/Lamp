#include "GraphKey.h"

#include <imnodes.h>

#include <Lamp/Objects/Entity/Base/ComponentRegistry.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <imgui/imgui_stdlib.h>

#include <Lamp/GraphKey/Link.h>
#include <Lamp/GraphKey/NodeRegistry.h>

namespace Sandbox3D
{
	using namespace Lamp;

	static uint64_t s_Ids = 0;

	GraphKey::GraphKey(std::string_view name)
		: BaseWindow(name)
	{
		CreateComponentNodes();
		CreateRegistryNodes();
	}

	void GraphKey::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(GraphKey::UpdateImGui));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(GraphKey::OnUpdate));
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
		}

		return true;
	}

	void GraphKey::UpdateNodeWindow()
	{
		ImGui::Begin("Main", &m_IsOpen);



		imnodes::BeginNodeEditor();

		for (auto& node : m_ExistingNodes)
		{
			DrawNode(node);
		}

		//Draw Links
		for (auto& link : m_Links)
		{
			imnodes::Link(link.id, link.start, link.end);
		}

		imnodes::EndNodeEditor();

		int startAttr, endAttr;
		if (imnodes::IsLinkCreated(&startAttr, &endAttr))
		{
			Node* pStartNode = nullptr;
			Node* pEndNode = nullptr;

			Attribute* pStartAttr = nullptr;
			Attribute* pEndAttr = nullptr;
			for (auto& node : m_ExistingNodes)
			{
				for (auto& attr : node->attributes)
				{
					if (attr.id == startAttr)
					{
						pStartNode = node.get();
						pStartAttr = &attr;
					}
					if (attr.id == endAttr)
					{
						pEndNode = node.get();
						pEndAttr = &attr;
					}

					if (pStartNode && pEndNode)
					{
						break;
					}
				}
			}

			Link l{ s_Ids++, startAttr, endAttr };

			if (pStartAttr && pEndAttr)
			{
				if (pStartAttr->property.PropertyType == pEndAttr->property.PropertyType)
				{
					pStartNode->links.push_back(l);
					pEndNode->links.push_back(l);

				}
			}
			m_Links.push_back(l);
		}

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
			for (auto& node : m_ComponentNodes)
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

				ImGui::TreeNodeEx((void*)i, nodeFlags, node->name.c_str());
				if (ImGui::IsItemClicked())
				{
					Ref<Node> n = CreateRef<Node>(node);
					n->id = s_Ids++;

					for (auto& attr : n->attributes)
					{
						attr.id = s_Ids++;
					}

					m_ExistingNodes.push_back(n);
				}

				i++;
			}

			ImGui::TreePop();
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

			for (auto& prop : pComp->GetComponentProperties().GetProperties())
			{
				Attribute attr;
				attr.type = Lamp::AttributeType::Input;
				attr.property = prop;

				node->attributes.push_back(attr);

			}

			m_ComponentNodes.push_back(node);
		}
	}

	void GraphKey::CreateRegistryNodes()
	{
		for (auto& node : NodeRegistry::s_Methods())
		{
			Ref<Lamp::Node> n = node.second();

			m_ComponentNodes.push_back(n);
		}
	}

	void GraphKey::RemoveNode(uint32_t id)
	{
		auto func = [&id](Ref<Node>& node)
		{
			return id == node->id;
		};

		m_ExistingNodes.erase(std::remove_if(m_ExistingNodes.begin(), m_ExistingNodes.end(), func), m_ExistingNodes.end());
	}

	void GraphKey::DrawNode(Ref<Lamp::Node>& node)
	{
		imnodes::BeginNode(node->id);

		imnodes::BeginNodeTitleBar();
		ImGui::Text(node->name.c_str());
		imnodes::EndNodeTitleBar();

		if (!node->pEntity)
		{
			ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "No entity assigned!");
		}

		for (auto attr : node->attributes)
		{
			auto pProp = attr.property;

			if (attr.type == Lamp::AttributeType::Input)
			{
				imnodes::BeginInputAttribute(attr.id);
			}
			else if (attr.type == Lamp::AttributeType::Output)
			{
				imnodes::BeginOutputAttribute(attr.id);
			}


			float nodeWidth = 100.f;
			if (pProp.PropertyType == Lamp::PropertyType::Float3 || pProp.PropertyType == Lamp::PropertyType::Float4
				|| pProp.PropertyType == Lamp::PropertyType::Color3 || pProp.PropertyType == Lamp::PropertyType::Color4)
			{
				nodeWidth = 200.f;
			}
			float labelWidth = ImGui::CalcTextSize(pProp.Name.c_str()).x;

			ImGui::PushItemWidth(nodeWidth - labelWidth);

			switch (pProp.PropertyType)
			{
				case Lamp::PropertyType::Int:
				{
					int* p = static_cast<int*>(pProp.Value);
					int v = *p;

					ImGui::DragInt(pProp.Name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}

					break;
				}

				case Lamp::PropertyType::Bool:
				{
					bool* p = static_cast<bool*>(pProp.Value);
					bool v = *p;

					ImGui::Checkbox(pProp.Name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}

					break;
				}

				case Lamp::PropertyType::Float:
				{
					float* p = static_cast<float*>(pProp.Value);
					float v = *p;

					ImGui::DragFloat(pProp.Name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}

				case Lamp::PropertyType::Float2:
				{
					glm::vec2* p = static_cast<glm::vec2*>(pProp.Value);
					glm::vec2 v = *p;

					ImGui::DragFloat2(pProp.Name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}

				case Lamp::PropertyType::Float3:
				{
					glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);
					glm::vec3 v = *p;

					ImGui::DragFloat3(pProp.Name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}

				case Lamp::PropertyType::Float4:
				{
					glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);
					glm::vec4 v = *p;

					ImGui::DragFloat4(pProp.Name.c_str(), glm::value_ptr(*p));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}

				case Lamp::PropertyType::String:
				{
					std::string* s = static_cast<std::string*>(pProp.Value);
					std::string v = *s;

					ImGui::InputText(pProp.Name.c_str(), &v);
					if (v != *s)
					{
						*s = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}

				case Lamp::PropertyType::Path:
				{
					std::string* s = static_cast<std::string*>(pProp.Value);
					std::string v = *s;

					ImGui::InputText(pProp.Name.c_str(), &v);
					ImGui::SameLine();
					if (ImGui::Button("Open..."))
					{
						std::string path = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
						if (!path.empty())
						{
							v = path;
						}
					}

					if (v != *s)
					{
						*s = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}

				case Lamp::PropertyType::Color3:
				{
					glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);
					glm::vec3 v = *p;

					ImGui::ColorEdit3(pProp.Name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}

				case Lamp::PropertyType::Color4:
				{
					glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);
					glm::vec4 v = *p;

					ImGui::ColorEdit4(pProp.Name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
					break;
				}
			}

			ImGui::PopItemWidth();

			if (attr.type == Lamp::AttributeType::Input)
			{
				imnodes::EndInputAttribute();
			}
			else if (attr.type == Lamp::AttributeType::Output)
			{
				imnodes::EndOutputAttribute();
			}
		}

		imnodes::EndNode();
	}
}