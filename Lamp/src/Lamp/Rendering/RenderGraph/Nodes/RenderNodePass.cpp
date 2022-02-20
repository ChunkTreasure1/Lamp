#include "lppch.h"
#include "RenderNodePass.h"

#include "RenderNodeEnd.h"
#include "RenderNodeCompute.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/RenderCommand.h"

#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/StandardUtilities.h"
#include "Lamp/Utility/UIUtility.h"
#include "Lamp/Utility/ImGuiExtension.h"

#include <imnodes.h>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Lamp
{
	static const GraphUUID targetBufferId = GraphUUID(1);
	static const uint32_t framebufferLocation = 1;
	static const std::string s_defaultShader = "pbrForward";

	namespace Utils
	{
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
		CreateDefaultRenderPipeline();

		Ref<RenderOutputAttribute> activated = CreateRef<RenderOutputAttribute>();
		activated->name = "Finished";
		activated->pNode = this;
		activated->type = RenderAttributeType::Pass;

		outputs.push_back(activated);

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->name = "Output";
		output->pNode = this;
		output->type = RenderAttributeType::Framebuffer;

		outputs.push_back(output);

		Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
		input->name = "Run";
		input->pNode = this;
		input->type = RenderAttributeType::Pass;
		input->shouldDraw = true;

		inputs.push_back(input);

		Ref<RenderInputAttribute> targetBuffer = CreateRef<RenderInputAttribute>();
		targetBuffer->name = "Target framebuffer";
		targetBuffer->pNode = this;
		targetBuffer->data = targetBufferId;
		targetBuffer->type = RenderAttributeType::Framebuffer;
		targetBuffer->shouldDraw = true;

		inputs.push_back(targetBuffer);
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
					if (auto passNode = reinterpret_cast<RenderNodePass*>(link->pInput->pNode))
					{
						auto& passSpec = const_cast<RenderPipelineSpecification&>(passNode->renderPass.graphicsPipeline->GetSpecification());
						GraphUUID id = std::any_cast<GraphUUID>(link->pInput->data);

					}
					else if (auto computeNode = reinterpret_cast<RenderNodeCompute*>(link->pInput->pNode))
					{
						computeNode->framebuffer = renderPass.graphicsPipeline->GetSpecification().framebuffer;
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

		auto& specification = const_cast<RenderPipelineSpecification&>(renderPass.graphicsPipeline->GetSpecification());

		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(74, 58, 232, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(64, 97, 255, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(64, 97, 255, 255));

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
		UI::InputText(nameId, specification.debugName);

		const float treeWidth = ImNodes::GetNodeDimensions(id).x - 20.f;

		if (UI::TreeNodeFramed("Settings", treeWidth))
		{
			DrawSettings();

			UI::TreeNodePop();
		}

		if (!IsAttributeLinked(inputs[framebufferLocation]))
		{
			if (UI::TreeNodeFramed("Output Framebuffer", treeWidth))
			{
				DrawOutputBuffer();

				UI::TreeNodePop();
			}
		}

		if (UI::TreeNodeFramed("Framebuffers", treeWidth))
		{
			DrawFramebuffers();

			UI::TreeNodePop();
		}
		else
		{
			for (auto& buffer : renderPass.graphicsPipeline->GetSpecification().framebufferInputs)
			{
				auto attr = FindAttributeByID(buffer.graphId);
				if (IsAttributeLinked(attr))
				{
					attr->shouldDraw = true;
				}
			}
		}

		DrawAttributes(inputs, outputs);

		ImGui::PopItemWidth();

		ImNodes::EndNode();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodePass::Activate(std::any value)
	{
		Ref<CameraBase> camera = std::any_cast<Ref<CameraBase>>(value);

		RenderCommand::BeginPass(renderPass.graphicsPipeline);
		RenderCommand::DispatchRenderCommands();
		RenderCommand::EndPass();

		for (const auto& link : links)
		{
			if (link->pInput->pNode->id == id || !link->pInput || !link->pInput->pNode)
			{
				continue;
			}

			switch (link->pOutput->type)
			{
				case RenderAttributeType::Pass:
				{
					if (link->pInput->pNode->GetNodeType() == RenderNodeType::End)
					{
						link->pInput->pNode->Activate(renderPass.graphicsPipeline->GetSpecification().framebuffer);
					}
					else
					{
						link->pInput->pNode->Activate(value);
					}
					break;
				}

				case RenderAttributeType::Framebuffer:
				{
					if (auto pass = dynamic_cast<RenderNodePass*>(link->pInput))
					{
						const_cast<RenderPipelineSpecification&>(pass->renderPass.graphicsPipeline->GetSpecification()).framebuffer = renderPass.graphicsPipeline->GetSpecification().framebuffer;
						pass->renderPass.graphicsPipeline->Invalidate();
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
		const auto& specification = renderPass.graphicsPipeline->GetSpecification();

		LP_SERIALIZE_PROPERTY(name, specification.debugName, out);
		LP_SERIALIZE_PROPERTY(drawType, (uint32_t)specification.drawType, out);
		LP_SERIALIZE_PROPERTY(cullFace, (uint32_t)specification.cullMode, out);
		LP_SERIALIZE_PROPERTY(shader, (specification.shader ? specification.shader->GetName() : ""), out);
		LP_SERIALIZE_PROPERTY(drawSkybox, specification.drawSkybox, out);
		LP_SERIALIZE_PROPERTY(drawTerrain, specification.drawTerrain, out);
		LP_SERIALIZE_PROPERTY(depthTest, specification.depthTest, out);
		LP_SERIALIZE_PROPERTY(depthWrite, specification.depthWrite, out);
		LP_SERIALIZE_PROPERTY(topology, (uint32_t)specification.topology, out);
		LP_SERIALIZE_PROPERTY(isSwapchain, specification.isSwapchain, out);
		LP_SERIALIZE_PROPERTY(useTessellation, specification.useTessellation, out);
		LP_SERIALIZE_PROPERTY(tessellationControlPoints, specification.tessellationControlPoints, out);

		out << YAML::Key << "vertexLayout" << YAML::Value;
		out << YAML::BeginMap;
		{
			LP_SERIALIZE_PROPERTY(stride, specification.vertexLayout.m_stride, out);

			out << YAML::Key << "elements" << YAML::BeginSeq;
			{
				for (const auto& element : specification.vertexLayout.m_elements)
				{
					out << YAML::BeginMap;
					{
						LP_SERIALIZE_PROPERTY(normalized, element.normalized, out);
						LP_SERIALIZE_PROPERTY(name, element.name, out);
						LP_SERIALIZE_PROPERTY(offset, element.offset, out);
						LP_SERIALIZE_PROPERTY(size, element.size, out);
						LP_SERIALIZE_PROPERTY(type, (uint32_t)element.type, out);
					}
					out << YAML::EndMap;
				}
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;

		out << YAML::Key << "framebuffer" << YAML::Value;
		out << YAML::BeginMap;
		{
			const auto& targetBuffSpec = specification.framebuffer->GetSpecification();

			LP_SERIALIZE_PROPERTY(useViewportSize, m_useViewportSize, out);
			LP_SERIALIZE_PROPERTY(width, targetBuffSpec.width, out);
			LP_SERIALIZE_PROPERTY(height, targetBuffSpec.height, out);
			LP_SERIALIZE_PROPERTY(samples, targetBuffSpec.samples, out);
			LP_SERIALIZE_PROPERTY(clearColor, targetBuffSpec.clearColor, out);
			LP_SERIALIZE_PROPERTY(copyable, targetBuffSpec.copyable, out);
			LP_SERIALIZE_PROPERTY(shadow, targetBuffSpec.shadow, out);
			LP_SERIALIZE_PROPERTY(swapchainTarget, targetBuffSpec.swapchainTarget, out);
			LP_SERIALIZE_PROPERTY(bindId, m_bindId, out);


			out << YAML::Key << "attachments" << YAML::BeginSeq;
			{
				for (auto& att : targetBuffSpec.attachments.attachments)
				{
					out << YAML::BeginMap;
					{
						out << YAML::Key << "attachment" << YAML::Value << att.name;

						LP_SERIALIZE_PROPERTY(borderColor, att.borderColor, out);
						LP_SERIALIZE_PROPERTY(multisampled, att.multisampled, out);
						LP_SERIALIZE_PROPERTY(format, (uint32_t)att.textureFormat, out);
						LP_SERIALIZE_PROPERTY(filtering, (uint32_t)att.textureFiltering, out);
						LP_SERIALIZE_PROPERTY(wrap, (uint32_t)att.textureWrap, out);
						LP_SERIALIZE_PROPERTY(blending, (uint32_t)att.blending, out);
						LP_SERIALIZE_PROPERTY(clearMode, (uint32_t)att.clearMode, out);
					}
					out << YAML::EndMap;

				}
			}
			out << YAML::EndSeq; //attachments
		}
		out << YAML::EndMap; //target framebuffer

		out << YAML::Key << "framebuffers" << YAML::BeginSeq;
		for (const auto& framebufferSpec : specification.framebufferInputs)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "framebuffer" << YAML::Value;

			LP_SERIALIZE_PROPERTY(guuid, framebufferSpec.id, out);
			LP_SERIALIZE_PROPERTY(attrId, framebufferSpec.attributeId, out);

			out << YAML::Key << "attachments" << YAML::BeginSeq;
			for (const auto& attachment : framebufferSpec.)
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

		SerializeAttributes(out);
	}

	void RenderNodePass::Deserialize(YAML::Node& node)
	{
		auto& specification = const_cast<RenderPipelineSpecification&>(renderPass.graphicsPipeline->GetSpecification());

		specification.debugName = node["name"].as<std::string>();
		specification.drawType = (DrawType)node["drawType"].as<uint32_t>();
		specification.cullMode = (CullMode)node["cullFace"].as<uint32_t>();
		specification.topology = (Topology)node["topology"].as<uint32_t>();

		LP_DESERIALIZE_PROPERTY(drawSkybox, specification.drawSkybox, node, false);
		LP_DESERIALIZE_PROPERTY(drawTerrain, specification.drawTerrain, node, false);
		LP_DESERIALIZE_PROPERTY(depthTest, specification.depthTest, node, true);
		LP_DESERIALIZE_PROPERTY(depthWrite, specification.depthWrite, node, true);
		LP_DESERIALIZE_PROPERTY(isSwapchain, specification.isSwapchain, node, false);
		LP_DESERIALIZE_PROPERTY(useTessellation, specification.useTessellation, node, false);
		LP_DESERIALIZE_PROPERTY(tessellationControlPoints, specification.tessellationControlPoints, node, 4);


		//Vertex layout
		{
			auto vertLayoutNode = node["vertexLayout"];
			BufferLayout layout;

			LP_DESERIALIZE_PROPERTY(stride, layout.m_stride, vertLayoutNode, 0);


			for (auto entry : vertLayoutNode)
			{
				bool normalized = false;
				std::string name = "";
				uint32_t offset = 0;
				size_t size = 0;
				ElementType type;

				LP_DESERIALIZE_PROPERTY(normalized, normalized, entry, false);
				LP_DESERIALIZE_PROPERTY(name, name, entry, "");
				LP_DESERIALIZE_PROPERTY(offset, offset, entry, 0);
				LP_DESERIALIZE_PROPERTY(size, size, entry, 0);

				type = (ElementType)node["type"].as<uint32_t>();

				BufferElement element{ type, name, normalized };
				element.offset = offset;
				element.size = size;

				layout.m_elements.emplace_back(element);
			}

			specification.vertexLayout = layout;
		}

		std::string shaderName = node["renderShader"].as<std::string>();
		if (!shaderName.empty())
		{
			specification.shader = ShaderLibrary::GetShader(shaderName);
		}

		YAML::Node bufferNode = node["framebuffer"];

		//target buffer
		auto& targetBufferSpec = specification.framebuffer->GetSpecification();
		LP_DESERIALIZE_PROPERTY(useViewportSize, m_useViewportSize, bufferNode, false);
		LP_DESERIALIZE_PROPERTY(width, targetBufferSpec.width, bufferNode, 0);
		LP_DESERIALIZE_PROPERTY(height, targetBufferSpec.height, bufferNode, 0);
		LP_DESERIALIZE_PROPERTY(samples, targetBufferSpec.samples, bufferNode, 0);
		LP_DESERIALIZE_PROPERTY(clearColor, targetBufferSpec.clearColor, bufferNode, glm::vec4(0.f));
		LP_DESERIALIZE_PROPERTY(copyable, targetBufferSpec.copyable, bufferNode, false);
		LP_DESERIALIZE_PROPERTY(shadow, targetBufferSpec.shadow, bufferNode, false);
		LP_DESERIALIZE_PROPERTY(swapchainTarget, targetBufferSpec.swapchainTarget, bufferNode, false);
		LP_DESERIALIZE_PROPERTY(bindId, m_bindId, bufferNode, 0);

		YAML::Node attachmentsNode = bufferNode["attachments"];

		for (auto entry : attachmentsNode)
		{
			FramebufferTextureSpecification att;

			LP_DESERIALIZE_PROPERTY(borderColor, att.borderColor, entry, glm::vec4(0.f));
			LP_DESERIALIZE_PROPERTY(multisampled, att.multisampled, entry, false);

			att.TextureFormat = (FramebufferTextureFormat)entry["format"].as<uint32_t>();
			att.TextureFiltering = (FramebufferTextureFiltering)entry["filtering"].as<uint32_t>();
			att.TextureWrap = (FramebufferTextureWrap)entry["wrap"].as<uint32_t>();
			att.name = entry["attachment"].as<std::string>();

			targetBufferSpec.Attachments.Attachments.push_back(att);
		}

		specification.targetFramebuffer = Framebuffer::Create(specification.targetFramebuffer->GetSpecification());

		if (m_useViewportSize)
		{
			Renderer::s_pSceneData->useViewportSize.push_back(specification.targetFramebuffer);
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

				attachmentSpecs.emplace_back(type, bindSlot, attachId);
			}

			specification.framebuffers.emplace_back(nullptr, attachmentSpecs, name, guuid, attrId);
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

	void RenderNodePass::CreateDefaultRenderPipeline()
	{
		auto shader = ShaderLibrary::GetShader(s_defaultShader);

		FramebufferSpecification framebufferSpec{};
		framebufferSpec.swapchainTarget = false;
		framebufferSpec.attachments =
		{
			ImageFormat::RGBA
		};

		bool hasUniformBuffers = false;
		bool hasShaderStorageBuffers = false;
		for (const auto& set : shader->GetDescriptorSets())
		{
			if (!set.uniformBuffers.empty())
			{
				hasUniformBuffers = true;
				break;
			}
		}

		for (const auto& set : shader->GetDescriptorSets())
		{
			if (!set.storageBuffers.empty())
			{
				hasShaderStorageBuffers = true;
				break;
			}
		}

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
		pipelineSpec.shader = shader;
		pipelineSpec.isSwapchain = false;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.uniformBufferSets = hasUniformBuffers ? Renderer::Get().GetStorage().uniformBufferSet : nullptr;
		pipelineSpec.shaderStorageBufferSets = hasShaderStorageBuffers ? Renderer::Get().GetStorage().shaderStorageBufferSet : nullptr;
		pipelineSpec.drawSkybox = false;
		pipelineSpec.drawTerrain = false;
		pipelineSpec.debugName = name;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" },
		};

		renderPass.graphicsPipeline = RenderPipeline::Create(pipelineSpec);
	}

	void RenderNodePass::DrawSettings()
	{
		auto& specification = const_cast<RenderPipelineSpecification&>(renderPass.graphicsPipeline->GetSpecification());

		ImGui::PushID("settings");
		uint32_t stackId = 0;

		const float distance = 75.f;

		//Shader
		{
			int currentlySelectedShader = 0;
			if (specification.shader)
			{
				auto it = std::find(m_shaders.begin(), m_shaders.end(), specification.shader->GetName().c_str());
				currentlySelectedShader = (int)std::distance(m_shaders.begin(), it);
			}

			if (Utils::DrawCombo("Shader", "##" + std::to_string(stackId++), m_shaders, currentlySelectedShader))
			{
				if (currentlySelectedShader == 0)
				{
					specification.shader = nullptr;
				}
				else
				{
					specification.shader = ShaderLibrary::GetShader(m_shaders[currentlySelectedShader]);
				}
			}
		}

		//Topology
		{
			static const std::vector<const char*> drawTypes = { "TriangleList", "LineList", "TriangleStrip", "PatchList" };
			int currentlySelectedDrawType = (int)specification.drawType;

			if (Utils::DrawCombo("Draw type", "##" + std::to_string(stackId++), drawTypes, currentlySelectedDrawType))
			{
				specification.drawType = (DrawType)currentlySelectedDrawType;
			}
		}

		//DrawType
		{
			static const std::vector<const char*> drawTypes = { "Opaque", "Transparent", "Translucency", "Quad", "Cube", "Terrain", "Skybox" };
			int currentlySelectedDrawType = (int)specification.drawType;

			if (Utils::DrawCombo("Draw type", "##" + std::to_string(stackId++), drawTypes, currentlySelectedDrawType))
			{
				specification.drawType = (DrawType)currentlySelectedDrawType;
			}
		}

		//Cull face
		{
			static const std::vector<const char*> cullFaces = { "Front", "Back", "FrontAndBack", "None" };
			int currentlySelectedCullFace = (int)specification.cullMode;

			if (Utils::DrawCombo("Cull face", "##" + std::to_string(stackId++), cullFaces, currentlySelectedCullFace))
			{
				specification.cullMode = (CullMode)currentlySelectedCullFace;
			}
		}

		//Skybox
		{
			float offset = distance - ImGui::CalcTextSize("Draw skybox").x;

			ImGui::TextUnformatted("Draw skybox");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &specification.drawSkybox);
		}

		//Terrain
		{
			float offset = distance - ImGui::CalcTextSize("Draw terrain").x;

			ImGui::TextUnformatted("Draw terrain");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &specification.drawTerrain);
		}

		//Depth Test
		{
			float offset = distance - ImGui::CalcTextSize("Depth Test").x;

			ImGui::TextUnformatted("Depth Test");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &specification.depthTest);
		}

		//Depth Write
		{
			float offset = distance - ImGui::CalcTextSize("Depth Write").x;

			ImGui::TextUnformatted("Depth Write");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &specification.depthWrite);
		}

		//Tessellation
		{
			float offset = distance - ImGui::CalcTextSize("Use Tessellation").x;

			ImGui::TextUnformatted("Use Tessellation");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::Checkbox(("##" + std::to_string(stackId++)).c_str(), &specification.useTessellation);

			if (specification.useTessellation)
			{
				offset = distance - ImGui::CalcTextSize("Control Points").x;

				ImGui::TextUnformatted("Control Points");
				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				ImGui::InputInt(("##" + std::to_string(stackId++)).c_str(), (int*)& specification.tessellationControlPoints);
			}
		}

		ImGui::PopID();
	}

	void RenderNodePass::DrawOutputBuffer()
	{
		auto& specification = renderPass.graphicsPipeline->GetSpecification().framebuffer->GetSpecification();
		const float maxWidth = 80.f;
		const float treeWidth = ImNodes::GetNodeDimensions(id).x - 20.f;

		//Viewport size
		{
			float offset = maxWidth - ImGui::CalcTextSize("Viewport size").x;

			ImGui::Text("Viewport size");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			if (ImGui::Checkbox(std::string("##viewpostSize").c_str(), &m_useViewportSize))
			{
				if (m_useViewportSize)
				{
					Renderer::s_pSceneData->useViewportSize.push_back(renderPass->GetSpecification().targetFramebuffer);
				}
				else
				{
					auto& vector = Renderer::s_pSceneData->useViewportSize;
					if (auto it = std::find(vector.begin(), vector.end(), renderPass->GetSpecification().targetFramebuffer); it != vector.end())
					{
						vector.erase(it);
					}
				}
			}
		}

		//Bind id
		{
			float offset = maxWidth - ImGui::CalcTextSize("Bind ID").x;

			ImGui::Text("Bind ID");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::InputInt("##bindId", &m_bindId);
		}

		//Size
		{
			float offset = maxWidth - ImGui::CalcTextSize("Height").x;
			if (!m_useViewportSize)
			{
				int width = static_cast<int>(specification.width);
				float offset = maxWidth - ImGui::CalcTextSize("Width").x;

				ImGui::TextUnformatted("Width");
				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				if (ImGui::InputInt("##Width", &width))
				{
					specification.width = width;
				}

				int height = static_cast<int>(specification.height);

				offset = maxWidth - ImGui::CalcTextSize("Height").x;

				ImGui::TextUnformatted("Height");
				ImGui::SameLine();
				UI::ShiftCursor(offset, 0.f);

				if (ImGui::InputInt("##Height", &height))
				{
					specification.height = height;
				}
			}

			int samples = static_cast<int>(specification.samples);
			offset = maxWidth - ImGui::CalcTextSize("Samples").x;

			ImGui::TextUnformatted("Samples");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			if (ImGui::InputInt("##samples", &samples))
			{
				specification.samples = samples;
			}
		}

		//Clear color
		{
			float offset = maxWidth - ImGui::CalcTextSize("Clear color").x;

			ImGui::TextUnformatted("Clear color");
			ImGui::SameLine();
			UI::ShiftCursor(offset, 0.f);

			ImGui::PushItemWidth(200.f);
			ImGui::ColorEdit4("##clearColor", glm::value_ptr(specification.clearColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);
			ImGui::PopItemWidth();
		}

		if (UI::TreeNodeFramed("Attachments", treeWidth - 20.f))
		{
			if (ImGui::Button("Add attachment"))
			{
				specification.attachments.attachments.emplace_back();
			}

			int attIndex = 0;
			for (auto& att : specification.attachments.attachments)
			{
				std::string attId = std::to_string(attIndex);
				bool changed = false;

				if (ImGui::Button(("-##" + attId).c_str()))
				{
					for (uint32_t i = 0; i < specification.attachments.attachments.size(); i++)
					{
						if (specification.attachments.attachments[i] == att)
						{
							specification.attachments.attachments.erase(specification.attachments.attachments.begin() + i);
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

						ImGui::ColorEdit4("##borderColor", glm::value_ptr(att.borderColor), ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB);
					}

					//Multi sampled
					{
						float offset = maxWidth - ImGui::CalcTextSize("Multisampled").x;
						ImGui::TextUnformatted("Multisampled");
						ImGui::SameLine();
						UI::ShiftCursor(offset, 0.f);
						ImGui::Checkbox("##multiSampled", &att.multisampled);
					}

					//Format
					{
						static const std::vector<const char*> formats = { "None", "RGBA8", "RGBA16F", "RGBA32F", "RG32F", "RED_INTEGER", "RED", "DEPTH32F", "DEPTH24STENCIL8" };
						int currentlySelectedFormat = (int)att.textureFormat;

						if (Utils::DrawCombo("Format", "##format" + attId, formats, currentlySelectedFormat))
						{
							att.textureFormat = (ImageFormat)currentlySelectedFormat;
							changed = true;
						}
					}

					//Filtering
					{
						static const std::vector<const char*> filtering = { "Nearest", "Linear", "NearestMipMapNearest", "LinearMipMapNearest", "NearestMipMapLinear", "LinearMipMapLinear" };
						int currentlySelectedFiltering = (int)att.textureFiltering;

						if (Utils::DrawCombo("Filtering", "##filtering" + attId, filtering, currentlySelectedFiltering))
						{
							att.textureFiltering = (TextureFilter)currentlySelectedFiltering;
							changed = true;
						}
					}

					{
						static const std::vector<const char*> wrap = { "Repeat", "MirroredRepeat", "ClampToEdge", "ClampToBorder", "MirrorClampToEdge" };
						int currentlySelectedWrap = (int)att.textureWrap;

						if (Utils::DrawCombo("Wrap", "##wrap" + attId, wrap, currentlySelectedWrap))
						{
							att.textureWrap = (TextureWrap)currentlySelectedWrap;
							changed = true;
						}
					}

					if (changed)
					{
						renderPass.graphicsPipeline->GetSpecification().framebuffer->Invalidate();
					}


					ImGui::TreePop();
				}
				attIndex++;
			}

			ImGui::TreePop();
		}
	}

	void RenderNodePass::DrawFramebuffers()
	{
		auto& specification = const_cast<RenderPipelineSpecification&>(renderPass.graphicsPipeline->GetSpecification());

		if (ImGui::Button("Add##framebuffer"))
		{
			Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();

			PassFramebufferSpecification buffSpec;
			buffSpec.id = GraphUUID();
			buffSpec.attributeId = input->id;

			specification.framebuffers.push_back(buffSpec);

			input->data = buffSpec.id;
			input->pNode = this;
			input->name = "Framebuffer" + std::to_string(specification.framebuffers.size() - 1);
			input->type = RenderAttributeType::Framebuffer;

			inputs.push_back(input);
		}

		for (auto& framebufferSpec : specification.framebuffers)
		{
			ImGui::PushID(framebufferSpec.name.c_str());
			uint32_t stackId = 0;

			ImNodesPinShape pinShape = IsAttributeLinked(FindAttributeByID(framebufferSpec.attributeId)) ? ImNodesPinShape_TriangleFilled : ImNodesPinShape_Triangle;

			ImNodes::PushColorStyle(ImNodesCol_Pin, Utils::GetTypeColor(RenderAttributeType::Framebuffer));
			ImNodes::PushColorStyle(ImNodesCol_PinHovered, Utils::GetTypeHoverColor(RenderAttributeType::Framebuffer));

			FindAttributeByID(framebufferSpec.attributeId)->shouldDraw = false;
			ImNodes::BeginInputAttribute(framebufferSpec.attributeId, pinShape);

			if (ImGui::Button(("-##" + std::to_string(stackId++)).c_str()))
			{
				RemoveAttribute(RenderAttributeType::Framebuffer, framebufferSpec.attributeId);
				if (Utility::RemoveFromContainer(specification.framebuffers, framebufferSpec))
				{
					ImNodes::EndInputAttribute();
					ImGui::PopID();
					break;
				}
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