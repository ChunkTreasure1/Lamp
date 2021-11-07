#include "lppch.h"
#include "RenderNodeFramebuffer.h"

#include "RenderNodePass.h"
#include "RenderNodeCompute.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Utility/UIUtility.h"
#include "Lamp/Rendering/RenderGraph/RenderGraphUtils.h"

#include <imnodes.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <memory>

namespace Lamp
{
	namespace Utils
	{
		static TextureType TextureFormatToType(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case Lamp::FramebufferTextureFormat::None: return TextureType::Color;
				case Lamp::FramebufferTextureFormat::RGBA8: return TextureType::Color;
				case Lamp::FramebufferTextureFormat::RGBA16F: return TextureType::Color;
				case Lamp::FramebufferTextureFormat::RGBA32F: return TextureType::Color;
				case Lamp::FramebufferTextureFormat::RG32F: return TextureType::Color;
				case Lamp::FramebufferTextureFormat::RED_INTEGER: return TextureType::Color;
				case Lamp::FramebufferTextureFormat::RED: return TextureType::Color;
				case Lamp::FramebufferTextureFormat::DEPTH32F: return TextureType::Depth;
				case Lamp::FramebufferTextureFormat::DEPTH24STENCIL8: return TextureType::Depth;
			}
		}
	}

	void RenderNodeFramebuffer::Initialize()
	{
		framebuffer = Framebuffer::Create(FramebufferSpecification());

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->pNode = this;
		output->name = "Output";
		output->type = RenderAttributeType::Framebuffer;

		outputs.push_back(output);

		for (const auto& [uName, uBuffer] : Renderer::s_pSceneData->internalFramebuffers)
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
				auto& renderPassSpec = const_cast<RenderPassSpecification&>(passNode->renderPass->GetSpecification());

				auto buffer = Utils::GetSpecificationById<PassFramebufferSpecification>(renderPassSpec.framebuffers, id);
				buffer->framebuffer = framebuffer;

				int idOffset = 0;
				for (auto& att : framebuffer->GetSpecification().Attachments.Attachments)
				{
					buffer->attachments.emplace_back(Utils::TextureFormatToType(att.TextureFormat), m_bindId + idOffset, idOffset);
				}
			}
			else if (RenderNodeCompute* computeNode = dynamic_cast<RenderNodeCompute*>(link->pInput->pNode))
			{
				computeNode->framebuffer = framebuffer;
			}
		}
	}

	void RenderNodeFramebuffer::DrawNode()
	{
		LP_PROFILE_FUNCTION();

		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 28, 17, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(179, 53, 41, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(179, 53, 41, 255));

		ImGui::PushID(("framebuffer" + std::to_string(id)).c_str());
		uint32_t stackId = 0;

		ImNodes::BeginNode(id);

		ImGui::PushItemWidth(200.f);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("Framebuffer node");
		ImNodes::EndNodeTitleBar();

		ImGui::PushItemWidth(200.f);

		const float maxOffset = 90.f;

		float offset = 130.f - ImGui::CalcTextSize("Internal framebuffer").x;
		ImGui::TextUnformatted("Internal framebuffer");
		
		ImGui::SameLine();
		UI::ShiftCursor(offset, 0.f);

		if (ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &m_useInternalBuffers))
		{
			if (!m_useInternalBuffers)
			{
				FramebufferSpecification spec;
				framebuffer = Framebuffer::Create(spec);
			}
			else
			{
				framebuffer = Renderer::s_pSceneData->internalFramebuffers[m_BufferNames[m_CurrentlySelectedBuffer]];
				m_SelectedBufferName = m_BufferNames[m_CurrentlySelectedBuffer];
			}
		}

		offset = 50.f - ImGui::CalcTextSize("Bind slot").x;
		ImGui::TextUnformatted("Bind slot");

		ImGui::SameLine();
		UI::ShiftCursor(offset, 0.f);
		ImGui::PushItemWidth(100.f);
		ImGui::InputInt(("##" + std::to_string(stackId++)).c_str(), &m_bindId);
		ImGui::PopItemWidth();

		if (m_useInternalBuffers)
		{
			ImGui::PushItemWidth(158.f);
			if (ImGui::Combo("##buffers", &m_CurrentlySelectedBuffer, m_BufferNames.data(), (int)m_BufferNames.size()))
			{
				framebuffer = Renderer::s_pSceneData->internalFramebuffers[m_BufferNames[m_CurrentlySelectedBuffer]];
				m_SelectedBufferName = m_BufferNames[m_CurrentlySelectedBuffer];
			}
			ImGui::PopItemWidth();
		}
		else
		{
			const float treeWidth = ImNodes::GetNodeDimensions(id).x - 15.f;

			if (UI::TreeNodeFramed("Settings", treeWidth))
			{
				offset = maxOffset - ImGui::CalcTextSize("Viewport size").x;
				ImGui::TextUnformatted("Viewport size");

				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				if (ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &m_useScreenSize))
				{
					if (m_useScreenSize)
					{
						Renderer::s_pSceneData->useViewportSize.push_back(framebuffer);
					}
					else
					{
						auto& vector = Renderer::s_pSceneData->useViewportSize;
						if (auto it = std::find(vector.begin(), vector.end(), framebuffer); it != vector.end())
						{
							vector.erase(it);
						}
					}
				}

				auto& specification = framebuffer->GetSpecification();
				if (!m_useScreenSize)
				{
					int width = static_cast<int>(specification.Width);

					offset = maxOffset - ImGui::CalcTextSize("Width").x;
					ImGui::TextUnformatted("Width");

					ImGui::SameLine();
					UI::ShiftCursor(offset, 0.f);

					if (ImGui::InputInt(("##" + std::to_string(stackId++)).c_str(), &width))
					{
						specification.Width = width;
					}

					offset = maxOffset - ImGui::CalcTextSize("Height").x;
					ImGui::TextUnformatted("Height");

					ImGui::SameLine();
					UI::ShiftCursor(offset, 0.f);

					int height = static_cast<int>(specification.Height);
					if (ImGui::InputInt(("##" + std::to_string(stackId++)).c_str(), &height))
					{
						specification.Height = height;
					}
				}

				int samples = static_cast<int>(specification.Samples);
				offset = maxOffset - ImGui::CalcTextSize("Samples").x;
				ImGui::TextUnformatted("Samples");

				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				if (ImGui::InputInt(("##" + std::to_string(stackId++)).c_str(), &samples))
				{
					specification.Samples = samples;
				}

				offset = maxOffset - ImGui::CalcTextSize("Clear color").x;
				ImGui::TextUnformatted("Clear color");

				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				ImGui::ColorEdit4(("##" + std::to_string(stackId++)).c_str(), glm::value_ptr(specification.ClearColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);

				UI::TreeNodePop();
			}

			/*if (ImGui::TreeNode("Attachments"))
			{
				if (ImGui::Button("Add"))
				{
					specification.Attachments.Attachments.emplace_back();
				}

				ImGui::PushItemWidth(200.f);

				int attIndex = 0;
				for (auto& att : specification.Attachments.Attachments)
				{
					std::string attId = std::to_string(attIndex);
					bool changed = false;

					std::string treeId = "Attachment##" + attId;
					if (ImGui::TreeNode(treeId.c_str()))
					{
						ImGui::ColorEdit4("Border Color", glm::value_ptr(att.BorderColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);

						ImGui::Checkbox("Is multisampled", &att.MultiSampled);

						static const char* texFormats[] = { "None", "RGBA8", "RGBA16F", "RGBA32F", "RG32F", "RED_INTEGER", "RED", "DEPTH32F", "DEPTH24STENCIL8" };
						int currentlySelectedFormat = (int)att.TextureFormat;
						std::string formatId = "Texture format##format" + attId;
						if (ImGui::Combo(formatId.c_str(), &currentlySelectedFormat, texFormats, IM_ARRAYSIZE(texFormats)))
						{
							att.TextureFormat = (FramebufferTextureFormat)currentlySelectedFormat;
							changed = true;
						}

						static const char* texFiltering[] = { "Nearest", "Linear", "NearestMipMapNearest", "LinearMipMapNearest", "NearestMipMapLinear", "LinearMipMapLinear" };
						int currentlySelectedFiltering = (int)att.TextureFiltering;
						std::string filteringId = "Texture filtering##filtering" + attId;
						if (ImGui::Combo(filteringId.c_str(), &currentlySelectedFiltering, texFiltering, IM_ARRAYSIZE(texFiltering)))
						{
							att.TextureFiltering = (FramebufferTexureFiltering)currentlySelectedFiltering;
							changed = true;
						}

						static const char* texWrap[] = { "Repeat", "MirroredRepeat", "ClampToEdge", "ClampToBorder", "MirrorClampToEdge" };
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
			}*/
		}

		DrawAttributes(inputs, outputs);
		ImGui::PopItemWidth();
		ImNodes::EndNode();

		ImGui::PopID();

		ImGui::PopItemWidth();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeFramebuffer::Serialize(YAML::Emitter& out)
	{
		const auto& specification = framebuffer->GetSpecification();

		LP_SERIALIZE_PROPERTY(usingInternal, m_useInternalBuffers, out);
		LP_SERIALIZE_PROPERTY(selectedBuffer, m_SelectedBufferName, out);
		LP_SERIALIZE_PROPERTY(useViewportSize, m_useScreenSize, out);
		LP_SERIALIZE_PROPERTY(bindSlot, m_bindId, out);

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
		LP_DESERIALIZE_PROPERTY(usingInternal, m_useInternalBuffers, node, false);
		m_SelectedBufferName = node["selectedBuffer"].as<std::string>();
		LP_DESERIALIZE_PROPERTY(bindSlot, m_bindId, node, 0);

		if (m_useInternalBuffers)
		{
			framebuffer = Renderer::s_pSceneData->internalFramebuffers[m_SelectedBufferName];
		}
		else
		{
			auto& specification = framebuffer->GetSpecification();
			LP_DESERIALIZE_PROPERTY(useViewportSize, m_useScreenSize, node, false);
			LP_DESERIALIZE_PROPERTY(width, specification.Width, node, 0);
			LP_DESERIALIZE_PROPERTY(height, specification.Height, node, 0);
			LP_DESERIALIZE_PROPERTY(samples, specification.Samples, node, 0);
			LP_DESERIALIZE_PROPERTY(clearColor, specification.ClearColor, node, glm::vec4(0.f));

			if (m_useScreenSize)
			{
				Renderer::s_pSceneData->useViewportSize.push_back(framebuffer);
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