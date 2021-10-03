#include "lppch.h"
#include "RenderNodeFramebuffer.h"

#include "RenderNodePass.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

#include <imnodes.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <memory>

namespace Lamp
{
	void RenderNodeFramebuffer::Initialize()
	{
		framebuffer = Framebuffer::Create(FramebufferSpecification());

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->pNode = this;
		output->name = "Output";
		output->type = RenderAttributeType::Framebuffer;

		outputs.push_back(output);

		for (const auto &[uName, uBuffer] : Renderer3D::GetSettings().InternalFramebuffers)
		{
			m_BufferNames.push_back(uName.c_str());
		}
	}

	void RenderNodeFramebuffer::Start()
	{
		for (auto& link : links)
		{
			if (RenderNodePass* passNode = dynamic_cast<RenderNodePass*>(link->pInput->pNode))
			{
				GraphUUID id = std::any_cast<GraphUUID>(link->pInput->data);
				passNode->renderPass->GetSpecification().framebuffers[id].first.framebuffer = framebuffer;
			}
		}
	}

	void RenderNodeFramebuffer::DrawNode()
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 28, 17, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(179, 53, 41, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(179, 53, 41, 255));

		ImNodes::BeginNode(id);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("Framebuffer node");
		ImNodes::EndNodeTitleBar();

		ImGui::PushItemWidth(100.f);

		if (ImGui::Checkbox("Use internal framebuffer", &m_UseInternalBuffers))
		{
			if (!m_UseInternalBuffers)
			{
				FramebufferSpecification spec;
				framebuffer = Framebuffer::Create(spec);
			}
			else
			{
				framebuffer = Renderer3D::GetSettings().InternalFramebuffers[m_BufferNames[m_CurrentlySelectedBuffer]];
				m_SelectedBufferName = m_BufferNames[m_CurrentlySelectedBuffer];
			}
		}

		if (m_UseInternalBuffers)
		{
			ImGui::PushItemWidth(150.f);
			if (ImGui::Combo("##buffers", &m_CurrentlySelectedBuffer, m_BufferNames.data(), (int)m_BufferNames.size()))
			{
				framebuffer = Renderer3D::GetSettings().InternalFramebuffers[m_BufferNames[m_CurrentlySelectedBuffer]];
				m_SelectedBufferName = m_BufferNames[m_CurrentlySelectedBuffer];
			}
			ImGui::PopItemWidth();
		}
		else
		{
			if (ImGui::Checkbox("Use viewport size", &m_UseScreenSize))
			{
				if (m_UseScreenSize)
				{
					Renderer3D::GetSettings().UseViewportSize.push_back(framebuffer);
				}
				else
				{
					auto& vector = Renderer3D::GetSettings().UseViewportSize;
					if (auto it = std::find(vector.begin(), vector.end(), framebuffer); it != vector.end())
					{
						vector.erase(it);
					}
				}
			}

			auto& specification = framebuffer->GetSpecification();
			if (!m_UseScreenSize)
			{
				int width = static_cast<int>(specification.Width);
				if (ImGui::InputInt("Width", &width))
				{
					specification.Width = width;
				}

				int height = static_cast<int>(specification.Height);
				if (ImGui::InputInt("Height", &height))
				{
					specification.Height = height;
				}
			}

			int samples = static_cast<int>(specification.Samples);
			if (ImGui::InputInt("Samples", &samples))
			{
				specification.Samples = samples;
			}

			ImGui::PushItemWidth(200.f);
			ImGui::ColorEdit4("Clear Color", glm::value_ptr(specification.ClearColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);
			ImGui::PopItemWidth();

			if (ImGui::TreeNode("Attachments"))
			{
				if (ImGui::Button("Add"))
				{
					specification.Attachments.Attachments.push_back(FramebufferTextureSpecification());
				}

				ImGui::PushItemWidth(200.f);

				int attIndex = 0;
				for (auto &att : specification.Attachments.Attachments)
				{
					std::string attId = std::to_string(attIndex);
					bool changed = false;

					std::string treeId = "Attachment##" + attId;
					if (ImGui::TreeNode(treeId.c_str()))
					{
						ImGui::ColorEdit4("Border Color", glm::value_ptr(att.BorderColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);

						ImGui::Checkbox("Is multisampled", &att.MultiSampled);

						static const char *texFormats[] = { "None", "RGBA8", "RGBA16F", "RGBA32F", "RG32F", "RED_INTEGER", "RED", "DEPTH32F", "DEPTH24STENCIL8" };
						int currentlySelectedFormat = (int)att.TextureFormat;
						std::string formatId = "Texture format##format" + attId;
						if (ImGui::Combo(formatId.c_str(), &currentlySelectedFormat, texFormats, IM_ARRAYSIZE(texFormats)))
						{
							att.TextureFormat = (FramebufferTextureFormat)currentlySelectedFormat;
							changed = true;
						}

						static const char *texFiltering[] = { "Nearest", "Linear", "NearestMipMapNearest", "LinearMipMapNearest", "NearestMipMapLinear", "LinearMipMapLinear" };
						int currentlySelectedFiltering = (int)att.TextureFiltering;
						std::string filteringId = "Texture filtering##filtering" + attId;
						if (ImGui::Combo(filteringId.c_str(), &currentlySelectedFiltering, texFiltering, IM_ARRAYSIZE(texFiltering)))
						{
							att.TextureFiltering = (FramebufferTexureFiltering)currentlySelectedFiltering;
							changed = true;
						}

						static const char *texWrap[] = { "Repeat", "MirroredRepeat", "ClampToEdge", "ClampToBorder", "MirrorClampToEdge" };
						int currentlySelectedWrap = (int)att.TextureWrap;
						std::string wrapId = "Texture wrap##wrap" + attId;
						if (ImGui::Combo(wrapId.c_str(), &currentlySelectedWrap, texWrap, IM_ARRAYSIZE(texWrap)))
						{
							att.TextureWrap = (FramebufferTextureWrap)currentlySelectedWrap;
							changed = true;
						}

						if (changed)
						{
							framebuffer->Invalidate();
						}


						ImGui::TreePop();
					}
					attIndex++;
				}
				ImGui::PopItemWidth();

				ImGui::TreePop();
			}
		}

		DrawAttributes();

		ImNodes::EndNode();

		ImGui::PopItemWidth();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeFramebuffer::Serialize(YAML::Emitter& out)
	{
		const auto& specification = framebuffer->GetSpecification();

		LP_SERIALIZE_PROPERTY(usingInternal, m_UseInternalBuffers, out);
		LP_SERIALIZE_PROPERTY(selectedBuffer, m_SelectedBufferName, out);
		LP_SERIALIZE_PROPERTY(useViewportSize, m_UseScreenSize, out);

		LP_SERIALIZE_PROPERTY(width, specification.Width, out);
		LP_SERIALIZE_PROPERTY(height, specification.Height, out);
		LP_SERIALIZE_PROPERTY(samples, specification.Samples, out);
		LP_SERIALIZE_PROPERTY(clearColor, specification.ClearColor, out);
		
		out << YAML::Key << "attachments" << YAML::Value;
		out << YAML::BeginMap;
		{
			uint32_t attCount = 0;
			for (auto& att : specification.Attachments.Attachments)
			{
				out << YAML::Key << "attachment" + std::to_string(attCount) << YAML::Value;
				out << YAML::BeginMap;
				{
					LP_SERIALIZE_PROPERTY(borderColor, att.BorderColor, out);
					LP_SERIALIZE_PROPERTY(multisampled, att.MultiSampled, out);
					LP_SERIALIZE_PROPERTY(format, (uint32_t)att.TextureFormat, out);
					LP_SERIALIZE_PROPERTY(filtering, (uint32_t)att.TextureFiltering, out);
					LP_SERIALIZE_PROPERTY(wrap, (uint32_t)att.TextureWrap, out);
				}
				out << YAML::EndMap;

				attCount++;
			}
		}
		out << YAML::EndMap; //attachments

		framebuffer->Invalidate();

		SerializeAttributes(out);
	}

	void RenderNodeFramebuffer::Deserialize(YAML::Node& node)
	{
		LP_DESERIALIZE_PROPERTY(usingInternal, m_UseInternalBuffers, node, false);
		m_SelectedBufferName = node["selectedBuffer"].as<std::string>();

		if (m_UseInternalBuffers)
		{
			framebuffer = Renderer3D::GetSettings().InternalFramebuffers[m_SelectedBufferName];
		}
		else
		{
			auto &specification = framebuffer->GetSpecification();
			LP_DESERIALIZE_PROPERTY(useViewportSize, m_UseScreenSize, node, false);
			LP_DESERIALIZE_PROPERTY(width, specification.Width, node, 0);
			LP_DESERIALIZE_PROPERTY(height, specification.Height, node, 0);
			LP_DESERIALIZE_PROPERTY(samples, specification.Samples, node, 0);
			LP_DESERIALIZE_PROPERTY(clearColor, specification.ClearColor, node, glm::vec4(0.f));
			
			if (m_UseScreenSize)
			{
				Renderer3D::GetSettings().UseViewportSize.push_back(framebuffer);
			}

			YAML::Node attachmentsNode = node["attachments"];
			uint32_t attachmentCount = 0;
			while (YAML::Node attachmentNode = attachmentsNode["attachment" + std::to_string(attachmentCount)])
			{
				FramebufferTextureSpecification att;

				LP_DESERIALIZE_PROPERTY(borderColor, att.BorderColor, attachmentNode, glm::vec4(0.f));
				LP_DESERIALIZE_PROPERTY(multisampled, att.MultiSampled, attachmentNode, false);

				att.TextureFormat = (FramebufferTextureFormat)attachmentNode["format"].as<uint32_t>();
				att.TextureFiltering = (FramebufferTexureFiltering)attachmentNode["filtering"].as<uint32_t>();
				att.TextureWrap = (FramebufferTextureWrap)attachmentNode["wrap"].as<uint32_t>();

				specification.Attachments.Attachments.push_back(att);
				attachmentCount++;
			}
			framebuffer->Invalidate();
		}

		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}
}