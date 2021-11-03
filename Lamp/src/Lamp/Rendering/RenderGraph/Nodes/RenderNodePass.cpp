#include "lppch.h"
#include "RenderNodePass.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "RenderNodeEnd.h"
#include "RenderNodeCompute.h"

#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Utility/StandardUtilities.h"
#include "Lamp/Utility/UIUtility.h"
#include "Lamp/Utility/ImGuiExtension.h"
#include "Lamp/Rendering/RenderGraph/RenderGraphUtils.h"

#include <imnodes.h>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Lamp
{
	static const GraphUUID targetBufferId = GraphUUID(1);

	namespace Utils
	{
		static std::any GetResetValue(UniformType type)
		{
			switch (type)
			{
				case UniformType::Int:
					return 0;
				case UniformType::Float:
					return 0.f;
				case UniformType::Float2:
					return glm::vec2(0.f);
				case UniformType::Float3:
					return glm::vec3(0.f);
				case UniformType::Float4:
					return glm::vec4(0.f);
				case UniformType::Mat3:
					return glm::mat3(1.f);
				case UniformType::Mat4:
					return glm::mat4(1.f);
				case UniformType::Sampler2D:
					return 0;
				case UniformType::SamplerCube:
					return 0;
				case UniformType::RenderData:
					return RenderData::Transform;
				default:
					return -1;
			}
		}

		static bool DrawCombo(const std::string& text, const std::string& id, const std::vector<const char*>& data, int& selected)
		{
			const float maxWidth = 75.f;

			float offset = maxWidth - ImGui::CalcTextSize(text.c_str()).x;

			ImGui::TextUnformatted(text.c_str());
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			return ImGui::Combo(id.c_str(), &selected, data.data(), data.size());
		}
	}

	void RenderNodePass::Initialize()
	{
		renderPass = CreateRef<RenderPass>();
		FramebufferSpecification spec;
		const_cast<RenderPassSpecification&>(renderPass->GetSpecification()).TargetFramebuffer = Framebuffer::Create(spec);

		Ref<RenderOutputAttribute> activated = CreateRef<RenderOutputAttribute>();
		activated->name = "Finished";
		activated->pNode = this;
		activated->type = RenderAttributeType::Pass;

		outputs.push_back(activated);

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->name = "Framebuffer";
		output->pNode = this;
		output->type = RenderAttributeType::Framebuffer;

		outputs.push_back(output);

		Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
		input->name = "Run";
		input->pNode = this;
		input->type = RenderAttributeType::Pass;

		m_runAttribute = input;
		inputs.push_back(input);

		m_targetBufferAttribute = CreateRef<RenderInputAttribute>();
		m_targetBufferAttribute->name = "Target framebuffer";
		m_targetBufferAttribute->pNode = this;
		m_targetBufferAttribute->data = targetBufferId;
		m_targetBufferAttribute->type = RenderAttributeType::Framebuffer;

		inputs.push_back(m_targetBufferAttribute);
	}

	void RenderNodePass::Start()
	{
		for (const auto& link : links)
		{
			if (link->pInput->pNode->id == id)
			{
				continue;
			}

			switch (link->pInput->type)
			{
				case RenderAttributeType::Framebuffer:
				{

					if (RenderNodePass* passNode = dynamic_cast<RenderNodePass*>(link->pInput->pNode))
					{
						auto& passSpecification = const_cast<RenderPassSpecification&>(passNode->renderPass->GetSpecification());
						GraphUUID id = std::any_cast<GraphUUID>(link->pInput->data);
						if (id == 1)
						{
							passSpecification.TargetFramebuffer = renderPass->GetSpecification().TargetFramebuffer;
						}
						else
						{
							if (passSpecification.framebuffers.find(id) != passSpecification.framebuffers.end())
							{
								passSpecification.framebuffers[id].first.framebuffer = renderPass->GetSpecification().TargetFramebuffer;
							}

							if (passSpecification.framebufferCommands.find(id) != passSpecification.framebufferCommands.end())
							{
								passSpecification.framebufferCommands[id].first.secondary = renderPass->GetSpecification().TargetFramebuffer;
							}
						}
					}

					if (RenderNodeCompute* computeNode = dynamic_cast<RenderNodeCompute*>(link->pInput->pNode))
					{
						computeNode->framebuffer = renderPass->GetSpecification().TargetFramebuffer;
					}

					break;
				}

				default:
					break;
			}
		}
	}

	void RenderNodePass::DrawNode()
	{
		LP_PROFILE_FUNCTION();

		m_shaders.clear();
		m_shaders.push_back("None");
		for (auto& shader : ShaderLibrary::GetShaders())
		{
			m_shaders.push_back(shader->GetName().c_str());
		}

		auto& specification = const_cast<RenderPassSpecification&>(renderPass->GetSpecification());

		ImNodes::BeginNode(id);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();

		ImGui::Text("Render pass");
		ImNodes::EndNodeTitleBar();

		std::string nodeId = std::to_string(id);

		ImGui::PushItemWidth(200.f);

		float offset = 50.f - ImGui::CalcTextSize("Name").x;

		ImGui::TextUnformatted("Name");
		ImGui::SameLine();
		UI::ShiftCursor(offset, 0.f);

		std::string nameId = "##name" + nodeId;
		UI::InputText(nameId, specification.Name);

		const float treeWidth = ImNodes::GetNodeDimensions(id).x - 20.f;

		if (UI::TreeNodeFramed("Settings", treeWidth))
		{
			DrawSettings();

			UI::TreeNodePop();
		}

		if (!IsAttributeLinked(m_targetBufferAttribute))
		{
			if (UI::TreeNodeFramed("Output Framebuffer", treeWidth))
			{
				DrawOutputBuffer();

				UI::TreeNodePop();
			}
		}

		if (UI::TreeNodeFramed("Uniforms", treeWidth))
		{
			bool showUniforms = specification.renderShader != nullptr && !specification.renderShader->GetSpecification().uniforms.empty();

			if (!showUniforms)
			{
				ImGui::TextColored(ImVec4(0.874, 0.165, 0.164, 1.f), "There are no uniforms to show!");
			}
			else
			{
				DrawUniforms();
			}

			UI::TreeNodePop();
		}
		else
		{
			for (auto& uniformPair : renderPass->m_passSpecification.uniforms)
			{
				FindAttributeByID(uniformPair.second.second)->shouldDraw = true;
			}
		}

		if (UI::TreeNodeFramed("Textures", treeWidth))
		{
			DrawTextures();

			UI::TreeNodePop();
		}
		else
		{
			for (auto& texturePair : renderPass->m_passSpecification.textures)
			{
				FindAttributeByID(texturePair.second.second)->shouldDraw = true;
			}
		}

		if (UI::TreeNodeFramed("Framebuffers", treeWidth))
		{
			DrawFramebuffers();

			UI::TreeNodePop();
		}
		else
		{
			for (auto& bufferPair : renderPass->m_passSpecification.framebuffers)
			{
				FindAttributeByID(bufferPair.second.second)->shouldDraw = true;
			}
		}

		DrawAttributes({ m_runAttribute, m_targetBufferAttribute }, outputs);

		for (const auto& input : inputs)
		{
			if (IsAttributeLinked(input) && input->shouldDraw)
			{
				unsigned int pinColor = ImNodes::GetStyle().Colors[ImNodesCol_Pin];
				unsigned int pinHoverColor = ImNodes::GetStyle().Colors[ImNodesCol_PinHovered];

				if (input->type != RenderAttributeType::Pass)
				{
					pinColor = Utils::GetTypeColor(input->type);
					pinHoverColor = Utils::GetTypeHoverColor(input->type);
				}

				ImNodes::PushColorStyle(ImNodesCol_Pin, pinColor);
				ImNodes::PushColorStyle(ImNodesCol_PinHovered, pinHoverColor);

				ImNodes::BeginInputAttribute(input->id, ImNodesPinShape_TriangleFilled);
				ImGui::TextUnformatted(input->name.c_str());
				ImNodes::EndInputAttribute();

				ImNodes::PopColorStyle();
				ImNodes::PopColorStyle();
			}
		}

		ImGui::PopItemWidth();

		ImNodes::EndNode();
	}

	void RenderNodePass::Activate(std::any value)
	{
		Ref<CameraBase> camera = std::any_cast<Ref<CameraBase>>(value);
		renderPass->Render(camera);

		for (const auto& link : links)
		{
			if (link->pInput->pNode->id == id)
			{
				continue;
			}

			if (link->pInput == nullptr)
			{
				LP_CORE_ERROR("Input attribute is null!");
				continue;
			}
			if (link->pInput->pNode == nullptr)
			{
				LP_CORE_ERROR("Input node is null!");
				continue;
			}

			switch (link->pInput->pNode->GetNodeType())
			{
				case RenderNodeType::Pass:
				{
					if (link->pInput->type == RenderAttributeType::Pass)
					{
						link->pInput->pNode->Activate(value);
					}
					break;
				}

				case RenderNodeType::Compute:
				{
					if (link->pInput->type == RenderAttributeType::Pass)
					{
						link->pInput->pNode->Activate(value);
					}
					break;
				}

				case RenderNodeType::End:
				{
					if (link->pInput->type == RenderAttributeType::Pass)
					{
						link->pInput->pNode->Activate(renderPass->GetSpecification().TargetFramebuffer);
					}
					break;
				}

				default:
					break;
			}
		}
	}

	void RenderNodePass::Serialize(YAML::Emitter& out)
	{
		const auto& specification = renderPass->GetSpecification();

		LP_SERIALIZE_PROPERTY(name, specification.Name, out);
		LP_SERIALIZE_PROPERTY(clearType, (uint32_t)specification.clearType, out);
		LP_SERIALIZE_PROPERTY(drawType, (uint32_t)specification.drawType, out);
		LP_SERIALIZE_PROPERTY(cullFace, (uint32_t)specification.cullFace, out);
		LP_SERIALIZE_PROPERTY(renderShader, (specification.renderShader ? specification.renderShader->GetName() : ""), out);
		LP_SERIALIZE_PROPERTY(draw2D, specification.draw2D, out);
		LP_SERIALIZE_PROPERTY(drawSkybox, specification.drawSkybox, out);

		out << YAML::Key << "targetFramebuffer" << YAML::Value;
		out << YAML::BeginMap;
		{
			const auto& targetBuffSpec = specification.TargetFramebuffer->GetSpecification();

			LP_SERIALIZE_PROPERTY(useViewportSize, m_UseViewportSize, out);
			LP_SERIALIZE_PROPERTY(width, targetBuffSpec.Width, out);
			LP_SERIALIZE_PROPERTY(height, targetBuffSpec.Height, out);
			LP_SERIALIZE_PROPERTY(samples, targetBuffSpec.Samples, out);
			LP_SERIALIZE_PROPERTY(clearColor, targetBuffSpec.ClearColor, out);

			out << YAML::Key << "attachments" << YAML::BeginSeq;
			{
				for (auto& att : targetBuffSpec.Attachments.Attachments)
				{
					out << YAML::BeginMap;
					{
						out << YAML::Key << "attachment" << YAML::Value << "";

						LP_SERIALIZE_PROPERTY(borderColor, att.BorderColor, out);
						LP_SERIALIZE_PROPERTY(multisampled, att.MultiSampled, out);
						LP_SERIALIZE_PROPERTY(format, (uint32_t)att.TextureFormat, out);
						LP_SERIALIZE_PROPERTY(filtering, (uint32_t)att.TextureFiltering, out);
						LP_SERIALIZE_PROPERTY(wrap, (uint32_t)att.TextureWrap, out);
					}
					out << YAML::EndMap;

				}
			}
			out << YAML::EndSeq; //attachments
		}
		out << YAML::EndMap; //target framebuffer

		out << YAML::Key << "staticUniforms" << YAML::BeginSeq;
		{
			for (const auto& staticUniformPair : specification.staticUniforms)
			{
				const auto& staticUniformSpec = staticUniformPair.second;

				out << YAML::BeginMap;
				out << YAML::Key << "staticUniform" << YAML::Value << staticUniformSpec.name;

				LP_SERIALIZE_PROPERTY(guuid, staticUniformPair.first, out);
				LP_SERIALIZE_PROPERTY(type, (uint32_t)staticUniformSpec.type, out);

				switch (staticUniformSpec.type)
				{
					case UniformType::Int:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<int>(staticUniformSpec.data), out);
						break;
					}

					case UniformType::Float:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<float>(staticUniformSpec.data), out);
						break;
					}

					case UniformType::Float2:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec2>(staticUniformSpec.data), out);
						break;
					}

					case UniformType::Float3:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec3>(staticUniformSpec.data), out);
						break;
					}

					case UniformType::Float4:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec4>(staticUniformSpec.data), out);
						break;
					}

					case UniformType::Mat3:
					{
						break;
					}

					case UniformType::Mat4:
					{
						break;
					}

					case UniformType::Sampler2D:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<int>(staticUniformSpec.data), out);
						break;
					}

					case UniformType::SamplerCube:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<int>(staticUniformSpec.data), out);
						break;
					}

					case UniformType::RenderData:
					{
						LP_SERIALIZE_PROPERTY(data, (uint32_t)std::any_cast<RenderData>(staticUniformSpec.data), out);
						break;
					}
				}

				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq; //static uniforms

		out << YAML::Key << "dynamicUniforms" << YAML::BeginSeq;
		{
			for (const auto& dynUniformPair : specification.dynamicUniforms)
			{
				const auto& dynUniformSpec = dynUniformPair.second.first;

				out << YAML::BeginMap;
				out << YAML::Key << "dynamicUniform" << YAML::Value << dynUniformSpec.name;

				LP_SERIALIZE_PROPERTY(guuid, dynUniformPair.first, out);
				LP_SERIALIZE_PROPERTY(type, (uint32_t)dynUniformSpec.type, out);
				LP_SERIALIZE_PROPERTY(attrId, dynUniformPair.second.second, out);

				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq; //dynamic uniforms

		out << YAML::Key << "textures" << YAML::BeginSeq;
		{
			uint32_t texCount = 0;
			for (const auto& texturePair : specification.textures)
			{
				const auto& textureSpec = texturePair.second.first;

				out << YAML::BeginMap;
				out << YAML::Key << "texture" << YAML::Value << textureSpec.name;

				LP_SERIALIZE_PROPERTY(guuid, texturePair.first, out);
				LP_SERIALIZE_PROPERTY(bindSlot, textureSpec.bindSlot, out);
				LP_SERIALIZE_PROPERTY(attrId, texturePair.second.second, out);

				out << YAML::EndMap;
				texCount++;
			}
		}
		out << YAML::EndSeq; //textures

		out << YAML::Key << "framebuffers" << YAML::BeginSeq;
		for (const auto& framebufferPair : specification.framebuffers)
		{
			const auto& framebufferSpec = framebufferPair.second.first;

			out << YAML::BeginMap;
			out << YAML::Key << "framebuffer" << YAML::Value << framebufferSpec.name;

			LP_SERIALIZE_PROPERTY(guuid, framebufferPair.first, out);
			LP_SERIALIZE_PROPERTY(attrId, framebufferPair.second.second, out);

			out << YAML::Key << "attachments" << YAML::BeginSeq;
			for (const auto& attachment : framebufferSpec.attachments)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "attachment" << YAML::Value << "";
				LP_SERIALIZE_PROPERTY(textureType, (uint32_t)attachment.type, out);
				LP_SERIALIZE_PROPERTY(bindId, attachment.bindId, out);
				LP_SERIALIZE_PROPERTY(attachmentId, attachment.attachmentId, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		out << YAML::EndSeq; //framebuffers

		out << YAML::Key << "framebufferCommands" << YAML::BeginSeq;
		for (const auto& commandPair : specification.framebufferCommands)
		{
			const auto& commandSpec = commandPair.second.first;

			out << YAML::BeginMap;
			out << YAML::Key << "command" << YAML::Value << commandSpec.name;

			LP_SERIALIZE_PROPERTY(guuid, commandPair.first, out);
			LP_SERIALIZE_PROPERTY(attrId, commandPair.second.second, out);
			LP_SERIALIZE_PROPERTY(commandType, (uint32_t)commandSpec.command, out);

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		SerializeAttributes(out);
	}

	void RenderNodePass::Deserialize(YAML::Node& node)
	{
		auto& specification = const_cast<RenderPassSpecification&>(renderPass->GetSpecification());

		specification.Name = node["name"].as<std::string>();
		specification.clearType = (ClearType)node["clearType"].as<uint32_t>();
		specification.drawType = (DrawType)node["drawType"].as<uint32_t>();
		specification.cullFace = (CullFace)node["cullFace"].as<uint32_t>();
		LP_DESERIALIZE_PROPERTY(draw2D, specification.draw2D, node, false);
		LP_DESERIALIZE_PROPERTY(drawSkybox, specification.drawSkybox, node, false);

		std::string shaderName = node["renderShader"].as<std::string>();
		if (!shaderName.empty())
		{
			specification.renderShader = ShaderLibrary::GetShader(shaderName);
		}

		YAML::Node bufferNode = node["targetFramebuffer"];

		//target buffer
		auto& targetBufferSpec = specification.TargetFramebuffer->GetSpecification();
		LP_DESERIALIZE_PROPERTY(useViewportSize, m_UseViewportSize, bufferNode, false);
		LP_DESERIALIZE_PROPERTY(width, targetBufferSpec.Width, bufferNode, 0);
		LP_DESERIALIZE_PROPERTY(height, targetBufferSpec.Height, bufferNode, 0);
		LP_DESERIALIZE_PROPERTY(samples, targetBufferSpec.Samples, bufferNode, 0);
		LP_DESERIALIZE_PROPERTY(clearColor, targetBufferSpec.ClearColor, bufferNode, glm::vec4(0.f));

		YAML::Node attachmentsNode = bufferNode["attachments"];

		for (auto entry : attachmentsNode)
		{
			FramebufferTextureSpecification att;

			LP_DESERIALIZE_PROPERTY(borderColor, att.BorderColor, entry, glm::vec4(0.f));
			LP_DESERIALIZE_PROPERTY(multisampled, att.MultiSampled, entry, false);

			att.TextureFormat = (FramebufferTextureFormat)entry["format"].as<uint32_t>();
			att.TextureFiltering = (FramebufferTexureFiltering)entry["filtering"].as<uint32_t>();
			att.TextureWrap = (FramebufferTextureWrap)entry["wrap"].as<uint32_t>();

			targetBufferSpec.Attachments.Attachments.push_back(att);
		}

		specification.TargetFramebuffer = Framebuffer::Create(specification.TargetFramebuffer->GetSpecification());

		if (m_UseViewportSize)
		{
			Renderer::s_pSceneData->useViewportSize.push_back(specification.TargetFramebuffer);
		}

		//static uniforms
		YAML::Node staticUniformsNode = node["staticUniforms"];

		for (const auto entry : staticUniformsNode)
		{
			std::string uName = entry["staticUniform"].as<std::string>();
			UniformType uType = (UniformType)entry["type"].as<uint32_t>();
			GraphUUID guuid = entry["guuid"].as<GraphUUID>();
			std::any uData;

			switch (uType)
			{
				case Lamp::UniformType::Int:
					LP_DESERIALIZE_PROPERTY(data, uData, entry, 0);
					break;

				case Lamp::UniformType::Float:
					LP_DESERIALIZE_PROPERTY(data, uData, entry, 0.f);
					break;

				case Lamp::UniformType::Float2:
					LP_DESERIALIZE_PROPERTY(data, uData, entry, glm::vec2(0.f));
					break;

				case Lamp::UniformType::Float3:
					LP_DESERIALIZE_PROPERTY(data, uData, entry, glm::vec3(0.f));
					break;

				case Lamp::UniformType::Float4:
					LP_DESERIALIZE_PROPERTY(data, uData, entry, glm::vec4(0.f));
					break;

				case Lamp::UniformType::Mat3:
					break;

				case Lamp::UniformType::Mat4:
					break;

				case Lamp::UniformType::Sampler2D:
					LP_DESERIALIZE_PROPERTY(data, uData, entry, 0);
					break;

				case Lamp::UniformType::SamplerCube:
					LP_DESERIALIZE_PROPERTY(data, uData, entry, 0);
					break;

				case Lamp::UniformType::RenderData:
				{
					uint32_t data;
					LP_DESERIALIZE_PROPERTY(data, data, entry, 0);
					uData = (RenderData)data;
					break;
				}

				default:
					break;
			}

			specification.staticUniforms.emplace(guuid, PassStaticUniformSpecification(uName, uType, uData));
		}

		//dynamic uniforms
		YAML::Node dynamicUniformsNode = node["dynamicUniforms"];
		for (const auto entry : dynamicUniformsNode)
		{
			std::string uName = entry["dynamicUniform"].as<std::string>();
			UniformType uType = (UniformType)entry["type"].as<uint32_t>();

			GraphUUID attrId;
			LP_DESERIALIZE_PROPERTY(attrId, attrId, entry, 0);

			GraphUUID guuid;
			LP_DESERIALIZE_PROPERTY(guuid, guuid, entry, 0);

			specification.dynamicUniforms.emplace(guuid, std::make_pair(PassDynamicUniformSpecification(uName, uType, nullptr), attrId));
		}

		//textures
		YAML::Node texturesNode = node["textures"];
		for (const auto entry : texturesNode)
		{
			uint32_t bindSlot;
			GraphUUID attrId;
			GraphUUID guuid;
			std::string name;
			LP_DESERIALIZE_PROPERTY(bindSlot, bindSlot, entry, 0);
			LP_DESERIALIZE_PROPERTY(attrId, attrId, entry, 0);
			LP_DESERIALIZE_PROPERTY(guuid, guuid, entry, 0);
			name = entry["texture"].as<std::string>();

			specification.textures.emplace(guuid, std::pair(PassTextureSpecification(nullptr, bindSlot, name), attrId));
		}

		//framebuffers
		YAML::Node framebuffersNode = node["framebuffers"];
		for (const auto entry : framebuffersNode)
		{
			GraphUUID attrId;
			GraphUUID guuid;
			std::string name;
			LP_DESERIALIZE_PROPERTY(attrId, attrId, entry, 0);
			LP_DESERIALIZE_PROPERTY(guuid, guuid, entry, 0);

			name = entry["framebuffer"].as<std::string>();

			std::vector<PassFramebufferAttachmentSpec> attachmentSpecs;
			YAML::Node attachmentsNode = entry["attachments"];
			for (const auto attachment : attachmentsNode)
			{
				TextureType type = (TextureType)attachment["textureType"].as<uint32_t>();
				uint32_t bindSlot;
				uint32_t attachId;

				LP_DESERIALIZE_PROPERTY(bindId, bindSlot, attachment, 0);
				LP_DESERIALIZE_PROPERTY(attachmentId, attachId, attachment, 0);

				attachmentSpecs.push_back({ type, bindSlot, attachId });
			}

			specification.framebuffers.emplace(guuid, std::make_pair(PassFramebufferSpecification(nullptr, attachmentSpecs, name), attrId));
		}

		//framebuffer commands
		YAML::Node framebufferCommandsNode = node["framebufferCommands"];
		for (const auto entry : framebufferCommandsNode)
		{
			std::string name = entry["command"].as<std::string>();
			GraphUUID guuid;
			GraphUUID attrId;
			FramebufferCommand command;

			LP_DESERIALIZE_PROPERTY(guuid, guuid, entry, (GraphUUID)0);
			LP_DESERIALIZE_PROPERTY(attrId, attrId, entry, (GraphUUID)0);
			command = (FramebufferCommand)entry["commandType"].as<uint32_t>();

			specification.framebufferCommands.emplace(guuid, std::make_pair(PassFramebufferCommandSpecification(specification.TargetFramebuffer, nullptr, command, name), attrId));
		}

		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}

	void RenderNodePass::RemoveAttribute(RenderAttributeType type, GraphUUID compId)
	{
		for (int i = 0; i < inputs.size(); i++)
		{
			if (inputs[i]->type != type)
			{
				continue;
			}

			if (compId == inputs[i]->id)
			{
				for (const auto& link : links)
				{
					if (link->pInput->id == inputs[i]->id)
					{
						link->markedForDelete = true;
						break;
					}
				}

				inputs.erase(inputs.begin() + i);

				int newIndex = 0;
				for (const auto& input : inputs)
				{
					if (input->type == RenderAttributeType::Texture)
					{
						input->data = newIndex;
						newIndex++;
					}
				}
				break;
			}
		}
	}

	void RenderNodePass::SetAttributeName(const std::string& name, GraphUUID id)
	{
		for (auto& input : inputs)
		{
			if (input->id == id)
			{
				input->name = name;
			}
		}
	}

	void RenderNodePass::SetupUniforms()
	{
		auto& renderPassSpec = renderPass->m_passSpecification;

		inputs.resize(inputs.size() - renderPassSpec.uniforms.size());
		renderPassSpec.uniforms.clear();

		for (const auto& uniform : renderPassSpec.renderShader->GetSpecification().uniforms)
		{
			PassUnifromSpecification spec{ uniform.name, uniform.type, uniform.data, uniform.id };


			GraphUUID uniformId = GraphUUID();
			Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();

			renderPassSpec.uniforms.emplace(uniformId, std::make_pair<>(spec, input->id));

			input->pNode = this;
			input->name = uniform.name;
			input->type = RenderAttributeType::DynamicUniform;
			input->data = uniformId;

			inputs.push_back(input);
		}
		//std::sort(renderPassSpec.uniforms.begin(), renderPassSpec.uniforms.end(), [](const std::pair<GraphUUID, std::pair<PassUnifromSpecification, GraphUUID>>& first, const std::pair<GraphUUID, std::pair<PassUnifromSpecification, GraphUUID>>& second)
		//	{
		//		return first.second.first.id < second.second.first.id;
		//	});
	}

	void RenderNodePass::DrawUniforms()
	{
		auto& passSpec = renderPass->m_passSpecification;
		auto& shaderSpec = const_cast<ShaderSpecification&>(passSpec.renderShader->GetSpecification());

		for (auto& uniformPair : passSpec.uniforms)
		{
			auto& uniform = uniformPair.second.first;

			ImGui::PushID(uniform.name.c_str());
			uint32_t id = 0;

			ImNodesPinShape pinShape = IsAttributeLinked(FindAttributeByID(uniformPair.second.second)) ? ImNodesPinShape_TriangleFilled : ImNodesPinShape_Triangle;

			ImNodes::PushColorStyle(ImNodesCol_Pin, Utils::GetTypeColor(RenderAttributeType::DynamicUniform));
			ImNodes::PushColorStyle(ImNodesCol_PinHovered, Utils::GetTypeHoverColor(RenderAttributeType::DynamicUniform));

			FindAttributeByID(uniformPair.second.second)->shouldDraw = false;
			ImNodes::BeginInputAttribute(uniformPair.second.second, pinShape);

			float offset = 90.f - ImGui::CalcTextSize(uniform.name.c_str()).x;
			ImGui::Text(uniform.name.c_str());

			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);
			ImGui::PushItemWidth(100.f);
			static const char* uniformTypes[] = { "Int", "Float", "Float2", "Float3", "Float4", "Mat3", "Mat4", "Sampler2D", "SamplerCube", "RenderData" };

			int currentlySelectedType = (int)uniform.type;

			std::string comboId = "##" + std::to_string(id++);
			if (ImGui::Combo(comboId.c_str(), &currentlySelectedType, uniformTypes, IM_ARRAYSIZE(uniformTypes)))
			{
				uniform.type = (UniformType)currentlySelectedType;
				uniform.data = Utils::GetResetValue(uniform.type);
			}

			ImGui::SameLine();

			if (uniform.data.has_value() && !IsAttributeLinked(FindAttributeByID(uniformPair.second.second)))
			{
				if (uniform.type != UniformType::RenderData)
				{
					std::string inputId = "##" + std::to_string(id++);
					switch (uniform.type)
					{
						case UniformType::Int: ImGui::InputInt(inputId.c_str(), &std::any_cast<int&>(uniform.data)); break;
						case UniformType::Float: ImGui::DragFloat(inputId.c_str(), &std::any_cast<float&>(uniform.data)); break;
						case UniformType::Float2: ImGui::DragFloat2(inputId.c_str(), glm::value_ptr(std::any_cast<glm::vec2&>(uniform.data))); break;
						case UniformType::Float3: ImGui::DragFloat3(inputId.c_str(), glm::value_ptr(std::any_cast<glm::vec3&>(uniform.data))); break;
						case UniformType::Float4: ImGui::DragFloat4(inputId.c_str(), glm::value_ptr(std::any_cast<glm::vec4&>(uniform.data))); break;
						case UniformType::Mat3: ImGui::Text("Mat3");  break;
						case UniformType::Mat4: ImGui::Text("Mat4"); break;
						case UniformType::Sampler2D: ImGui::InputInt(inputId.c_str(), &std::any_cast<int&>(uniform.data)); break;
						case UniformType::SamplerCube: ImGui::InputInt(inputId.c_str(), &std::any_cast<int&>(uniform.data)); break;
					}
				}
				else
				{
					static const char* renderDataTypes[] = { "Transform", "Data", "Material", "Id" };
					std::string dTypeId = "##" + std::to_string(id++);
					int currentlySelectedData = (int)std::any_cast<RenderData>(uniform.data);
					if (ImGui::Combo(dTypeId.c_str(), &currentlySelectedData, renderDataTypes, IM_ARRAYSIZE(renderDataTypes)))
					{
						uniform.data = (RenderData)currentlySelectedData;
					}
				}
			}

			ImGui::PopItemWidth();
			ImGui::PopID();

			ImNodes::EndInputAttribute();

			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
		}
	}

	void RenderNodePass::DrawSettings()
	{
		auto& specification = renderPass->m_passSpecification;

		ImGui::PushID("settings");
		uint32_t stackId = 0;

		const float distance = 75.f;

		//ClearType
		{
			static const std::vector<const char*> clearTypes = { "None", "Color", "Depth", "ColorDepth" };
			int currentlySelectedClearType = (int)specification.clearType;

			if (Utils::DrawCombo("Clear type", "##" + std::to_string(stackId++), clearTypes, currentlySelectedClearType))
			{
				specification.clearType = (ClearType)currentlySelectedClearType;
			}
		}

		//DrawType
		{
			static const std::vector<const char*> drawTypes = { "All", "Quad", "Line", "Forward" };
			int currentlySelectedDrawType = (int)specification.drawType;

			if(Utils::DrawCombo("Draw type", "##" + std::to_string(stackId++), drawTypes, currentlySelectedDrawType))
			{
				specification.drawType = (DrawType)currentlySelectedDrawType;
			}
		}

		//Cull face
		{
			static const std::vector<const char*> cullFaces = { "Front", "Back" };
			int currentlySelectedCullFace = (int)specification.cullFace;

			if (Utils::DrawCombo("Cull face", "##" + std::to_string(stackId++), cullFaces, currentlySelectedCullFace))
			{
				specification.cullFace = (CullFace)currentlySelectedCullFace;
			}
		}

		//Shader
		{
			int currentlySelectedShader = 0;
			if (specification.renderShader)
			{
				auto it = std::find(m_shaders.begin(), m_shaders.end(), specification.renderShader->GetName().c_str());
				currentlySelectedShader = (int)std::distance(m_shaders.begin(), it);
			}

			if (Utils::DrawCombo("Shader", "##" + std::to_string(stackId++), m_shaders, currentlySelectedShader))
			{
				if (currentlySelectedShader == 0)
				{
					specification.renderShader = nullptr;
				}
				else
				{
					specification.renderShader = ShaderLibrary::GetShader(m_shaders[currentlySelectedShader]);
					SetupUniforms();
				}
			}
		}

		//2D
		{
			float offset = distance - ImGui::CalcTextSize("Draw 2D").x;

			ImGui::TextUnformatted("Draw 2D");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &specification.draw2D);
		}

		//Skybox
		{
			float offset = distance - ImGui::CalcTextSize("Draw skybox").x;

			ImGui::TextUnformatted("Draw skybox");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &specification.drawSkybox);
		}

		ImGui::PopID();
	}

	void RenderNodePass::DrawOutputBuffer()
	{
		auto& specification = renderPass->GetSpecification().TargetFramebuffer->GetSpecification();
		const float maxWidth = 80.f;
		const float treeWidth = ImNodes::GetNodeDimensions(id).x - 20.f;

		//Viewport size
		{
			float offset = maxWidth - ImGui::CalcTextSize("Viewport size").x;

			ImGui::Text("Viewport size");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			if (ImGui::Checkbox(std::string("##viewpostSize").c_str(), &m_UseViewportSize))
			{
				if (m_UseViewportSize)
				{
					Renderer::s_pSceneData->useViewportSize.push_back(renderPass->GetSpecification().TargetFramebuffer);
				}
				else
				{
					auto& vector = Renderer::s_pSceneData->useViewportSize;
					if (auto it = std::find(vector.begin(), vector.end(), renderPass->GetSpecification().TargetFramebuffer); it != vector.end())
					{
						vector.erase(it);
					}
				}
			}
		}

		//Size
		{
			float offset = maxWidth - ImGui::CalcTextSize("Height").x;
			if (!m_UseViewportSize)
			{
				int width = static_cast<int>(specification.Width);
				float offset = maxWidth - ImGui::CalcTextSize("Width").x;

				ImGui::TextUnformatted("Width");
				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				if (ImGui::InputInt("##Width", &width))
				{
					specification.Width = width;
				}

				int height = static_cast<int>(specification.Height);

				offset = maxWidth - ImGui::CalcTextSize("Height").x;

				ImGui::TextUnformatted("Height");
				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				if (ImGui::InputInt("##Height", &height))
				{
					specification.Height = height;
				}
			}

			int samples = static_cast<int>(specification.Samples);
			offset = maxWidth - ImGui::CalcTextSize("Samples").x;

			ImGui::TextUnformatted("Samples");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			if (ImGui::InputInt("##samples", &samples))
			{
				specification.Samples = samples;
			}
		}

		//Clear color
		{
			float offset = maxWidth - ImGui::CalcTextSize("Clear color").x;

			ImGui::TextUnformatted("Clear color");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::PushItemWidth(200.f);
			ImGui::ColorEdit4("##clearColor", glm::value_ptr(specification.ClearColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);
			ImGui::PopItemWidth();
		}

		if (UI::TreeNodeFramed("Attachments", treeWidth - 20.f))
		{
			if (ImGui::Button("Add attachment"))
			{
				specification.Attachments.Attachments.emplace_back();
			}

			int attIndex = 0;
			for (auto& att : specification.Attachments.Attachments)
			{
				std::string attId = std::to_string(attIndex);
				bool changed = false;

				if (ImGui::Button(("-##" + attId).c_str()))
				{
					for (uint32_t i = 0; i < specification.Attachments.Attachments.size(); i++)
					{
						if (specification.Attachments.Attachments[i] == att)
						{
							specification.Attachments.Attachments.erase(specification.Attachments.Attachments.begin() + i);
							break;
						}
					}
				}

				ImGui::SameLine();

				ImVec2 cursorPos = ImGui::GetCursorPos();
				std::string treeId = m_renamingAttachmentSpec == &att ? "###att" + attId : att.name + "###att" + attId;
				bool open = UI::TreeNodeFramed(treeId, treeWidth - 60.f);
				if (UI::BeginPopup())
				{
					if (ImGui::MenuItem("Rename"))
					{
						m_renamingAttachmentSpec = &att;
					}

					UI::EndPopup();
				}

				if (m_renamingAttachmentSpec == &att)
				{
					ImGui::SameLine();
					ImGui::SetCursorPosX(cursorPos.x + ImGui::GetTreeNodeToLabelSpacing());
					std::string renameId = "###rename" + attId;

					ImGui::PushItemWidth(75.f);
					UI::ScopedColor background{ ImGuiCol_FrameBg, { 0.1f, 0.1f, 0.1f, 0.1f } };
					UI::InputText(renameId, m_renamingAttachmentSpec->name);

					if (m_renamingAttachmentSpec != m_lastRenamingAttachmentSpec)
					{
						ImGuiID widgetId = ImGui::GetCurrentWindow()->GetID(renameId.c_str());
						ImGui::SetFocusID(widgetId, ImGui::GetCurrentWindow());
						ImGui::SetKeyboardFocusHere(-1);
						m_lastRenamingAttachmentSpec = m_renamingAttachmentSpec;
					}
					if (!ImGui::IsItemFocused())
					{
						m_renamingAttachmentSpec = nullptr;
						m_lastRenamingAttachmentSpec = nullptr;
					}
					if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
					{
						m_renamingAttachmentSpec = nullptr;
						m_lastRenamingAttachmentSpec = nullptr;
					}

					ImGui::PopItemWidth();
				}

				if (open)
				{
					//Border color
					{
						float offset = maxWidth - ImGui::CalcTextSize("Border color").x;

						ImGui::TextUnformatted("Border color");
						ImGui::SameLine();
						UI::ShiftCursor(offset, 0.f);

						ImGui::ColorEdit4("##borderColor", glm::value_ptr(att.BorderColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);
					}

					//Multi sampled
					{
						float offset = maxWidth - ImGui::CalcTextSize("Multisampled").x;
						ImGui::TextUnformatted("Multisampled");
						ImGui::SameLine();
						UI::ShiftCursor(offset, 0.f);
						ImGui::Checkbox("##multiSampled", &att.MultiSampled);
					}

					//Format
					{
						static const std::vector<const char*> formats = { "None", "RGBA8", "RGBA16F", "RGBA32F", "RG32F", "RED_INTEGER", "RED", "DEPTH32F", "DEPTH24STENCIL8" };
						int currentlySelectedFormat = (int)att.TextureFormat;

						if (Utils::DrawCombo("Format", "##format" + attId, formats, currentlySelectedFormat))
						{
							att.TextureFormat = (FramebufferTextureFormat)currentlySelectedFormat;
							changed = true;
						}
					}

					//Filtering
					{
						static const std::vector<const char*> filtering = { "Nearest", "Linear", "NearestMipMapNearest", "LinearMipMapNearest", "NearestMipMapLinear", "LinearMipMapLinear" };
						int currentlySelectedFiltering = (int)att.TextureFiltering;

						if (Utils::DrawCombo("Filtering", "##filtering" + attId, filtering, currentlySelectedFiltering))
						{
							att.TextureFiltering = (FramebufferTexureFiltering)currentlySelectedFiltering;
							changed = true;
						}
					}

					{
						static const std::vector<const char*> wrap = { "Repeat", "MirroredRepeat", "ClampToEdge", "ClampToBorder", "MirrorClampToEdge" };
						int currentlySelectedWrap = (int)att.TextureWrap;

						if (Utils::DrawCombo("Wrap", "##wrap" + attId, wrap, currentlySelectedWrap))
						{
							att.TextureWrap = (FramebufferTextureWrap)currentlySelectedWrap;
							changed = true;
						}
					}

					if (changed)
					{
						renderPass->GetSpecification().TargetFramebuffer->Invalidate();
					}


					ImGui::TreePop();
				}
				attIndex++;
			}

			ImGui::TreePop();
		}
	}

	void RenderNodePass::DrawTextures()
	{
		auto& specification = renderPass->m_passSpecification;

		if (ImGui::Button("Add##texture"))
		{
			Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();

			GraphUUID texId = GraphUUID();
			specification.textures.emplace(texId, std::make_pair(PassTextureSpecification(), input->id));

			input->data = texId;
			input->pNode = this;
			input->name = "Texture" + std::to_string(specification.textures.size() - 1);
			input->type = RenderAttributeType::Texture;

			inputs.push_back(input);
		}

		for (auto& texturePair : specification.textures)
		{
			auto& textureSpec = texturePair.second.first;

			ImGui::PushID(textureSpec.name.c_str());
			uint32_t id = 0;

			ImNodesPinShape pinShape = IsAttributeLinked(FindAttributeByID(texturePair.second.second)) ? ImNodesPinShape_TriangleFilled : ImNodesPinShape_Triangle;

			ImNodes::PushColorStyle(ImNodesCol_Pin, Utils::GetTypeColor(RenderAttributeType::Texture));
			ImNodes::PushColorStyle(ImNodesCol_PinHovered, Utils::GetTypeHoverColor(RenderAttributeType::Texture));

			FindAttributeByID(texturePair.second.second)->shouldDraw = false;
			ImNodes::BeginInputAttribute(texturePair.second.second, pinShape);

			if (ImGui::Button(("-##" + std::to_string(id++)).c_str()))
			{
				RemoveAttribute(RenderAttributeType::Texture, texturePair.second.second);
				specification.textures.erase(texturePair.first);
				ImNodes::EndInputAttribute();
				ImGui::PopID();
				break;
			}

			ImGui::SameLine();

			ImGui::PushItemWidth(90.f);

			if (UI::InputText(("##" + std::to_string(id++)), textureSpec.name))
			{
				SetAttributeName(name, texturePair.second.second);
			}

			ImGui::SameLine();

			int currBindSlot = textureSpec.bindSlot;
			if (ImGui::InputInt("##bindSlot", &currBindSlot))
			{
				textureSpec.bindSlot = currBindSlot;
			}

			ImGui::PopItemWidth();
			ImNodes::EndInputAttribute();

			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();

			ImGui::PopID();
		}
	}

	void RenderNodePass::DrawFramebuffers()
	{
		auto& specification = renderPass->m_passSpecification;

		if (ImGui::Button("Add##framebuffer"))
		{
			Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();

			GraphUUID framebufferId = GraphUUID();
			specification.framebuffers.emplace(framebufferId, std::make_pair(PassFramebufferSpecification(), input->id));

			input->data = framebufferId;
			input->pNode = this;
			input->name = "Framebuffer" + std::to_string(specification.framebuffers.size() - 1);
			input->type = RenderAttributeType::Framebuffer;

			inputs.push_back(input);
		}

		for (auto& framebufferPair : specification.framebuffers)
		{
			auto& framebufferSpec = framebufferPair.second.first;

			ImGui::PushID(framebufferSpec.name.c_str());
			uint32_t stackId = 0;

			ImNodesPinShape pinShape = IsAttributeLinked(FindAttributeByID(framebufferPair.second.second)) ? ImNodesPinShape_TriangleFilled : ImNodesPinShape_Triangle;

			ImNodes::PushColorStyle(ImNodesCol_Pin, Utils::GetTypeColor(RenderAttributeType::Framebuffer));
			ImNodes::PushColorStyle(ImNodesCol_PinHovered, Utils::GetTypeHoverColor(RenderAttributeType::Framebuffer));

			FindAttributeByID(framebufferPair.second.second)->shouldDraw = false;
			ImNodes::BeginInputAttribute(framebufferPair.second.second, pinShape);

			if (ImGui::Button(("-##" + std::to_string(stackId++)).c_str()))
			{
				RemoveAttribute(RenderAttributeType::Framebuffer, framebufferPair.second.second);
				specification.framebuffers.erase(framebufferPair.first);
				ImNodes::EndInputAttribute();
				ImGui::PopID();
				break;
			}

			ImGui::SameLine();
			if (UI::TreeNodeFramed("Framebuffer##" + std::to_string(stackId++), ImNodes::GetNodeDimensions(id).x - 60.f))
			{
				UI::TreeNodePop();
			}

			ImNodes::EndInputAttribute();

			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();

			ImGui::PopID();
		}
	}
}