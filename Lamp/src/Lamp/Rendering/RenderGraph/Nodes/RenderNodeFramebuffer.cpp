#include "lppch.h"
#include "RenderNodeFramebuffer.h"

#include "RenderNodePass.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

#include <imnodes.h>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Lamp
{
	void RenderNodeFramebuffer::Initialize()
	{
		framebuffer = Framebuffer::Create(FramebufferSpecification());

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->pNode = this;
		output->name = "Output";
		output->id = ++currId;
		output->type = RenderAttributeType::Framebuffer;

		outputs.push_back(output);
	}

	void RenderNodeFramebuffer::Start()
	{
		for (auto& link : links)
		{
			if (RenderNodePass* passNode = dynamic_cast<RenderNodePass*>(link->pInput->pNode))
			{
				uint32_t index = std::any_cast<uint32_t>(link->pInput->data);
				auto& [buffer, type, bindId, attachId] = passNode->renderPass->GetSpecification().framebuffers[index];
				buffer = framebuffer;
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

		auto& specification = framebuffer->GetSpecification();
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
		}

		for (auto& output : outputs)
		{
			ImNodes::BeginOutputAttribute(output->id);
			ImGui::Text(output->name.c_str());
			ImNodes::EndOutputAttribute();
		}

		ImNodes::EndNode();

		ImGui::PopItemWidth();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeFramebuffer::Serialize(YAML::Emitter& out)
	{
		const auto& specification = framebuffer->GetSpecification();

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

		uint32_t attrId = 0;
		for (auto& input : inputs)
		{
			SerializeBaseAttribute(input, "input", out, attrId);
			attrId++;
		}

		for (auto& output : outputs)
		{
			SerializeBaseAttribute(output, "output", out, attrId);
			attrId++;
		}
	}

	void RenderNodeFramebuffer::Deserialize(YAML::Node& node)
	{
		auto& specification = framebuffer->GetSpecification();
		LP_DESERIALIZE_PROPERTY(width, specification.Width, node, 0);
		LP_DESERIALIZE_PROPERTY(height, specification.Height, node, 0);
		LP_DESERIALIZE_PROPERTY(samples, specification.Samples, node, 0);
		LP_DESERIALIZE_PROPERTY(clearColor, specification.ClearColor, node, glm::vec4(0.f));
		
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

		//attributes
		outputs.clear();
		inputs.clear();
		uint32_t attributeCount = 0;

		while (YAML::Node attribute = node["attribute" + std::to_string(attributeCount)])
		{
			const auto& [attr, attrType] = DeserializeBaseAttribute(attribute);
			attr->pNode = this;
			if (attrType == "input")
			{
				inputs.push_back(std::dynamic_pointer_cast<RenderInputAttribute>(attr));
			}
			else
			{
				outputs.push_back(std::dynamic_pointer_cast<RenderOutputAttribute>(attr));
			}
			
			currId = attr->id;
			attributeCount++;
		}
	}
}