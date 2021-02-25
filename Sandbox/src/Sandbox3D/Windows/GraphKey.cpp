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

	static NodeProperty ToNodeProperty(ComponentProperty& prop)
	{
		NodeProperty n;
		n.name = prop.Name;
		n.type = prop.PropertyType;
		n.value = prop.Value;

		return n;
	}

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
			imnodes::Link(link->id, link->output->id, link->input->id);
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

			Ref<Link> pL = CreateRef<Link>();
			pL->id = s_Ids++;
			pL->input = pEndAttr;
			pL->output = pStartAttr;

			if (pStartAttr && pEndAttr)
			{
				if (pStartAttr->property.type == pEndAttr->property.type)
				{
					pStartNode->links.push_back(pL);
					pEndNode->links.push_back(pL);

					pStartAttr->pLink = pL;
					pEndAttr->pLink = pL;

					m_Links.push_back(pL);
				}
			}
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
				attr.property = ToNodeProperty(prop);

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
			auto prop = attr.property;

			if (attr.type == Lamp::AttributeType::Input)
			{
				imnodes::BeginInputAttribute(attr.id);
			}
			else if (attr.type == Lamp::AttributeType::Output)
			{
				imnodes::BeginOutputAttribute(attr.id);
			}


			float nodeWidth = 100.f;
			if (prop.type == Lamp::PropertyType::Float3 || prop.type == Lamp::PropertyType::Float4
				|| prop.type == Lamp::PropertyType::Color3 || prop.type == Lamp::PropertyType::Color4)
			{
				nodeWidth = 200.f;
			}
			float labelWidth = ImGui::CalcTextSize(prop.name.c_str()).x;

			ImGui::PushItemWidth(nodeWidth - labelWidth);

			if (attr.type == Lamp::AttributeType::Input)
			{
				DrawInput(attr, node);
			}
			else if (attr.type == Lamp::AttributeType::Output)
			{
				DrawOutput(attr, node);
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

	void GraphKey::DrawInput(Lamp::Attribute& attr, Ref<Lamp::Node>& node)
	{
		auto& prop = attr.property;

		switch (prop.type)
		{
			case Lamp::PropertyType::Int:
			{
				int* p = static_cast<int*>(prop.value);

				if (!attr.pLink)
				{
					int v = *p;

					ImGui::DragInt(prop.name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(*p) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Bool:
			{
				bool* p = static_cast<bool*>(prop.value);

				if (!attr.pLink)
				{
					bool v = *p;

					ImGui::Checkbox(prop.name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(*p) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Float:
			{
				float* p = static_cast<float*>(prop.value);

				if (!attr.pLink)
				{
					float v = *p;

					ImGui::DragFloat(prop.name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(*p) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Float2:
			{
				glm::vec2* p = static_cast<glm::vec2*>(prop.value);

				if (attr.pLink)
				{
					glm::vec2 v = *p;
					ImGui::DragFloat2(prop.name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p->x) + ", " + std::to_string(p->y) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Float3:
			{
				glm::vec3* p = static_cast<glm::vec3*>(prop.value);

				if (!attr.pLink)
				{
					glm::vec3 v = *p;

					ImGui::DragFloat3(prop.name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p->x) + ", " + std::to_string(p->y) + ", " + std::to_string(p->z) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Float4:
			{
				glm::vec4* p = static_cast<glm::vec4*>(prop.value);

				if (!attr.pLink)
				{
					glm::vec4 v = *p;

					ImGui::DragFloat4(prop.name.c_str(), glm::value_ptr(*p));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p->x) + ", " + std::to_string(p->y) + ", " + std::to_string(p->z) + ", " + std::to_string(p->w) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::String:
			{
				std::string* s = static_cast<std::string*>(prop.value);

				if (!attr.pLink)
				{
					std::string v = *s;

					ImGui::InputText(prop.name.c_str(), &v);
					if (v != *s)
					{
						*s = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + *s + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Path:
			{
				std::string* s = static_cast<std::string*>(prop.value);

				if (!attr.pLink)
				{
					std::string v = *s;

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

					if (v != *s)
					{
						*s = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + *s + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Color3:
			{
				glm::vec3* p = static_cast<glm::vec3*>(prop.value);

				if (!attr.pLink)
				{
					glm::vec3 v = *p;

					ImGui::ColorEdit3(prop.name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p->x) + ", " + std::to_string(p->y) + ", " + std::to_string(p->z) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}
				break;
			}

			case Lamp::PropertyType::Color4:
			{
				glm::vec4* p = static_cast<glm::vec4*>(prop.value);

				if (!attr.pLink)
				{
					glm::vec4 v = *p;

					ImGui::ColorEdit4(prop.name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						if (node->pEntity)
						{
							Lamp::EntityPropertyChangedEvent e;
							node->pEntity->OnEvent(e);
						}
					}
				}
				else
				{
					ImGui::Text(std::string("(" + std::to_string(p->x) + ", " + std::to_string(p->y) + ", " + std::to_string(p->z) + ", " + std::to_string(p->w) + ")").c_str());
					ImGui::SameLine();
					ImGui::Text(prop.name.c_str());
				}

				break;
			}

			case Lamp::PropertyType::Void:
			{
				ImGui::Text(prop.name.c_str());

				break;
			}
		}
	}

	void GraphKey::DrawOutput(Lamp::Attribute& attr, Ref<Lamp::Node>& node)
	{
		auto& prop = attr.property;

		switch (prop.type)
		{
			case Lamp::PropertyType::Int:
			{
				int* p = static_cast<int*>(prop.value);
				int v = *p;

				ImGui::DragInt(prop.name.c_str(), &v);
				if (v != *p)
				{
					*p = v;

					if (attr.pLink)
					{
						int* s = static_cast<int*>(attr.pLink->input->property.value);
						*s = v;
					}

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
				bool* p = static_cast<bool*>(prop.value);
				bool v = *p;

				ImGui::Checkbox(prop.name.c_str(), &v);
				if (v != *p)
				{
					*p = v;

					if (attr.pLink)
					{
						bool* s = static_cast<bool*>(attr.pLink->input->property.value);
						*s = v;
					}

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
				float* p = static_cast<float*>(prop.value);
				float v = *p;

				ImGui::DragFloat(prop.name.c_str(), &v);
				if (v != *p)
				{
					*p = v;

					if (attr.pLink)
					{
						float* s = static_cast<float*>(attr.pLink->input->property.value);
						*s = v;
					}

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
				glm::vec2* p = static_cast<glm::vec2*>(prop.value);
				glm::vec2 v = *p;

				ImGui::DragFloat2(prop.name.c_str(), glm::value_ptr(v));
				if (v != *p)
				{
					*p = v;

					if (attr.pLink)
					{
						glm::vec2* s = static_cast<glm::vec2*>(attr.pLink->input->property.value);
						*s = v;
					}

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
				glm::vec3* p = static_cast<glm::vec3*>(prop.value);
				glm::vec3 v = *p;

				ImGui::DragFloat3(prop.name.c_str(), glm::value_ptr(v));
				if (v != *p)
				{
					*p = v;

					if (attr.pLink)
					{
						glm::vec3* s = static_cast<glm::vec3*>(attr.pLink->input->property.value);
						*s = v;
					}

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
				glm::vec4* p = static_cast<glm::vec4*>(prop.value);
				glm::vec4 v = *p;

				ImGui::DragFloat4(prop.name.c_str(), glm::value_ptr(*p));
				if (v != *p)
				{
					*p = v;
					if (attr.pLink)
					{
						glm::vec4* s = static_cast<glm::vec4*>(attr.pLink->input->property.value);
						*s = v;
					}

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
				std::string* s = static_cast<std::string*>(prop.value);
				std::string v = *s;

				ImGui::InputText(prop.name.c_str(), &v);
				if (v != *s)
				{
					*s = v;
					if (attr.pLink)
					{
						std::string* p = static_cast<std::string*>(attr.pLink->input->property.value);
						*p = v;
					}

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
				std::string* s = static_cast<std::string*>(prop.value);
				std::string v = *s;

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

				if (v != *s)
				{
					*s = v;
					if (attr.pLink)
					{
						std::string* p = static_cast<std::string*>(attr.pLink->input->property.value);
						*p = v;
					}

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
				glm::vec3* p = static_cast<glm::vec3*>(prop.value);

				glm::vec3 v = *p;

				ImGui::ColorEdit3(prop.name.c_str(), glm::value_ptr(v));
				if (v != *p)
				{
					*p = v;
					if (attr.pLink)
					{
						glm::vec3* s = static_cast<glm::vec3*>(attr.pLink->input->property.value);
						*s = v;
					}

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
				glm::vec4* p = static_cast<glm::vec4*>(prop.value);
				glm::vec4 v = *p;

				ImGui::ColorEdit4(prop.name.c_str(), glm::value_ptr(v));
				if (v != *p)
				{
					*p = v;
					if (attr.pLink)
					{
						glm::vec4* s = static_cast<glm::vec4*>(attr.pLink->input->property.value);
						*s = v;
					}

					if (node->pEntity)
					{
						Lamp::EntityPropertyChangedEvent e;
						node->pEntity->OnEvent(e);
					}
				}
				break;
			}

			case Lamp::PropertyType::Void:
			{
				ImGui::Text(prop.name.c_str());

				break;
			}
		}
	}
}