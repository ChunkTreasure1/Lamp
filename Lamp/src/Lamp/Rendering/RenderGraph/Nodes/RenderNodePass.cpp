#include "lppch.h"
#include "RenderNodePass.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "RenderNodeEnd.h"
#include "RenderNodeCompute.h"

#include "Lamp/Utility/StandardUtilities.h"
#include "Lamp/Utility/UIUtility.h"

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
	}

	void RenderNodePass::Initialize()
	{
		renderPass = CreateRef<RenderPass>();
		FramebufferSpecification spec;
		renderPass->GetSpecification().TargetFramebuffer = Framebuffer::Create(spec);

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->name = "Framebuffer";
		output->pNode = this;
		output->type = RenderAttributeType::Framebuffer;

		outputs.push_back(output);

		Ref<RenderOutputAttribute> activated = CreateRef<RenderOutputAttribute>();
		activated->name = "Activated";
		activated->pNode = this;
		activated->type = RenderAttributeType::Pass;

		outputs.push_back(activated);

		Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
		input->name = "Activate";
		input->pNode = this;
		input->type = RenderAttributeType::Pass;

		inputs.push_back(input);

		m_TargetBufferAttribute = CreateRef<RenderInputAttribute>();
		m_TargetBufferAttribute->name = "Target framebuffer";
		m_TargetBufferAttribute->pNode = this;
		m_TargetBufferAttribute->data = targetBufferId;
		m_TargetBufferAttribute->type = RenderAttributeType::Framebuffer;

		inputs.push_back(m_TargetBufferAttribute);
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
						GraphUUID id = std::any_cast<GraphUUID>(link->pInput->data);
						if (id == 1)
						{
							passNode->renderPass->GetSpecification().TargetFramebuffer = renderPass->GetSpecification().TargetFramebuffer;
						}
						else
						{
							if (passNode->renderPass->GetSpecification().framebuffers.find(id) != passNode->renderPass->GetSpecification().framebuffers.end())
							{
								passNode->renderPass->GetSpecification().framebuffers[id].first.framebuffer = renderPass->GetSpecification().TargetFramebuffer;
							}

							if (passNode->renderPass->GetSpecification().framebufferCommands.find(id) != passNode->renderPass->GetSpecification().framebufferCommands.end())
							{
								passNode->renderPass->GetSpecification().framebufferCommands[id].first.secondary = renderPass->GetSpecification().TargetFramebuffer;
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

		m_Shaders.clear();
		m_Shaders.push_back("None");
		for (auto& shader : ShaderLibrary::GetShaders())
		{
			m_Shaders.push_back(shader->GetName().c_str());
		}

		auto& specification = renderPass->GetSpecification();

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

		std::string nameId = "Name##node" + nodeId;
		ImGui::InputText(nameId.c_str(), &specification.Name);

		if (ImGui::TreeNode("Settings"))
		{
			//ClearType
			{
				static const char* clearTypes[] = { "None", "Color", "Depth", "ColorDepth" };
				int currentlySelectedClearType = (int)specification.clearType;
				std::string clearTypeId = "Clear Type##node" + nodeId;

				if (ImGui::Combo(clearTypeId.c_str(), &currentlySelectedClearType, clearTypes, 4))
				{
					specification.clearType = (ClearType)currentlySelectedClearType;
				}
			}

			//DrawType
			{
				static const char* drawTypes[] = { "All", "Quad", "Line", "Forward", "Deferred" };
				int currentlySelectedDrawType = (int)specification.drawType;
				std::string drawTypeId = "Draw Type##node" + nodeId;

				if (ImGui::Combo(drawTypeId.c_str(), &currentlySelectedDrawType, drawTypes, 5))
				{
					specification.drawType = (DrawType)currentlySelectedDrawType;
				}
			}

			//Cull face
			{
				static const char* cullFaces[] = { "Front", "Back" };
				int currentlySelectedCullFace = (int)specification.cullFace;
				std::string cullfaceId = "Cull Face##node" + nodeId;

				if (ImGui::Combo(cullfaceId.c_str(), &currentlySelectedCullFace, cullFaces, 2))
				{
					specification.cullFace = (CullFace)currentlySelectedCullFace;
				}
			}

			//Shader
			{
				int currentlySelectedShader = 0;
				if (specification.renderShader)
				{
					auto it = std::find(m_Shaders.begin(), m_Shaders.end(), specification.renderShader->GetName().c_str());
					currentlySelectedShader = (int)std::distance(m_Shaders.begin(), it);
				}
				std::string shaderId = "Render Shader##node" + nodeId;
				if (ImGui::Combo(shaderId.c_str(), &currentlySelectedShader, m_Shaders.data(), (int)m_Shaders.size()))
				{
					if (currentlySelectedShader == 0)
					{
						specification.renderShader = nullptr;
					}
					else
					{
						specification.renderShader = ShaderLibrary::GetShader(m_Shaders[currentlySelectedShader]);
					}
				}
			}

			ImGui::TreePop();
		}

		if (!IsAttributeLinked(m_TargetBufferAttribute))
		{
			if (ImGui::TreeNode("Target Framebuffer"))
			{
				auto& specification = renderPass->GetSpecification().TargetFramebuffer->GetSpecification();

				if (ImGui::Checkbox("Use viewport size", &m_UseViewportSize))
				{
					if (m_UseViewportSize)
					{
						Renderer3D::GetSettings().UseViewportSize.push_back(renderPass->GetSpecification().TargetFramebuffer);
					}
					else
					{
						auto& vector = Renderer3D::GetSettings().UseViewportSize;
						if (auto it = std::find(vector.begin(), vector.end(), renderPass->GetSpecification().TargetFramebuffer); it != vector.end())
						{
							vector.erase(it);
						}
					}
				}

				if (!m_UseViewportSize)
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
								renderPass->GetSpecification().TargetFramebuffer->Invalidate();
							}


							ImGui::TreePop();
						}
						attIndex++;
					}
					ImGui::PopItemWidth();

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}
		}

		if (ImGui::TreeNode("Static Uniforms"))
		{
			if (ImGui::Button("Create"))
			{
				specification.staticUniforms.emplace(GraphUUID(), PassStaticUniformSpecification());
			}

			uint32_t staticUniformCount = 0;
			for (auto& staticUniformPair : specification.staticUniforms)
			{
				auto& staticUniformSpec = staticUniformPair.second;

				std::string butId = "-##statRm" + std::to_string(staticUniformCount);
				if (ImGui::Button(butId.c_str()))
				{
					std::string sName = name;
					//Utility::RemoveFromContainerIf(specification.staticUniforms,
					//	[&sName](const std::tuple<std::string, UniformType, std::any>& t) { return std::get<0>(t) == sName; });
				}

				ImGui::SameLine();

				ImGui::PushItemWidth(100.f);

				std::string uNameId = "##uniformName" + std::to_string(staticUniformCount);
				ImGui::InputText(uNameId.c_str(), &staticUniformSpec.name);
				ImGui::SameLine();

				static const char* uTypes[] = { "Int", "Float", "Float2", "Float3", "Float4", "Mat3", "Mat4", "Sampler2D", "SamplerCube", "RenderData" };

				std::string uTypeId = "##uniformType" + std::to_string(staticUniformCount);
				int currentlySelectedType = (int)staticUniformSpec.type;
				if (ImGui::Combo(uTypeId.c_str(), &currentlySelectedType, uTypes, 10))
				{
					staticUniformSpec.type = (UniformType)currentlySelectedType;
					staticUniformSpec.data = Utils::GetResetValue(staticUniformSpec.type);
				}

				ImGui::PopItemWidth();

				ImGui::SameLine();

				ImGui::PushItemWidth(100.f);
				if (staticUniformSpec.data.has_value())
				{
					if (staticUniformSpec.type != UniformType::RenderData)
					{
						std::string inId = "##value" + std::to_string(staticUniformCount);
						switch (staticUniformSpec.type)
						{
							case UniformType::Int:
							{
								int data = std::any_cast<int>(staticUniformSpec.data);

								if (ImGui::InputInt(inId.c_str(), &data))
								{
									staticUniformSpec.data = data;
								}
								break;
							}

							case UniformType::Float:
							{
								float data = std::any_cast<float>(staticUniformSpec.data);
								if (ImGui::InputFloat(inId.c_str(), &data))
								{
									staticUniformSpec.data = data;
								}
								break;
							}

							case UniformType::Float2:
							{
								glm::vec2 data = std::any_cast<glm::vec2>(staticUniformSpec.data);
								if (ImGui::InputFloat2(inId.c_str(), glm::value_ptr(data)))
								{
									staticUniformSpec.data = data;
								}
								break;
							}

							case UniformType::Float3:
							{
								glm::vec3 data = std::any_cast<glm::vec3>(staticUniformSpec.data);
								if (ImGui::InputFloat3(inId.c_str(), glm::value_ptr(data)))
								{
									staticUniformSpec.data = data;
								}
								break;
							}

							case UniformType::Float4:
							{
								glm::vec4 data = std::any_cast<glm::vec4>(staticUniformSpec.data);
								if (ImGui::InputFloat4(inId.c_str(), glm::value_ptr(data)))
								{
									staticUniformSpec.data = data;
								}
								break;
							}

							case UniformType::Mat3:
							{
								glm::mat3 data = std::any_cast<glm::mat3>(staticUniformSpec.data);

								break;
							}

							case UniformType::Mat4:
							{
								glm::mat4 data = std::any_cast<glm::mat4>(staticUniformSpec.data);

								break;
							}

							case UniformType::Sampler2D:
							{
								int data = std::any_cast<int>(staticUniformSpec.data);
								if (ImGui::InputInt(inId.c_str(), &data))
								{
									staticUniformSpec.data = data;
								}
								break;
							}

							case UniformType::SamplerCube:
							{
								int data = std::any_cast<int>(staticUniformSpec.data);
								if (ImGui::InputInt(inId.c_str(), &data))
								{
									staticUniformSpec.data = data;
								}
								break;
							}

						}
					}
					else
					{
						static const char* dTypes[] = { "Transform", "Data", "Material", "Id" };
						std::string dTypeId = "##dataType" + std::to_string(staticUniformCount);
						int currentlySelectedData = (int)std::any_cast<RenderData>(staticUniformSpec.data);
						if (ImGui::Combo(dTypeId.c_str(), &currentlySelectedData, dTypes, 4))
						{
							staticUniformSpec.data = (RenderData)currentlySelectedData;
						}
					}
				}

				ImGui::PopItemWidth();

				staticUniformCount++;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Dynamic Uniforms"))
		{
			if (ImGui::Button("Create"))
			{
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();

				GraphUUID dynUniformId = GraphUUID();
				specification.dynamicUniforms.emplace(dynUniformId, std::make_pair(PassDynamicUniformSpecification(), input->id));

				input->pNode = this;
				input->name = "Uniform" + std::to_string(specification.dynamicUniforms.size() - 1);
				input->type = RenderAttributeType::DynamicUniform;
				input->data = dynUniformId;

				inputs.push_back(input);
			}

			uint32_t uniformCount = 0;
			for (auto& dynUniformPair : specification.dynamicUniforms)
			{
				auto& dynUniformSpec = dynUniformPair.second.first;

				std::string butId = "-##dynRm" + std::to_string(uniformCount);
				if (ImGui::Button(butId.c_str()))
				{
					RemoveAttribute(RenderAttributeType::DynamicUniform, dynUniformPair.second.second);

					std::string sName = name;
					specification.dynamicUniforms.erase(dynUniformPair.first);
				}

				ImGui::SameLine();

				ImGui::PushItemWidth(100.f);
				std::string nameId = "##dynUniformName" + std::to_string(uniformCount);
				if (ImGui::InputText(nameId.c_str(), &dynUniformSpec.name))
				{
					SetAttributeName(dynUniformSpec.name, dynUniformPair.second.second);
				}

				ImGui::PopItemWidth();
				uniformCount++;
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Textures"))
		{
			if (ImGui::Button("Add"))
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

			uint32_t texId = 0;
			for (auto& texturePair : specification.textures)
			{
				auto& textureSpec = texturePair.second.first;

				std::string texTreeId = "##tex" + std::to_string(texId);

				std::string butId = "-##texRm" + std::to_string(texId);
				if (ImGui::Button(butId.c_str()))
				{
					RemoveAttribute(RenderAttributeType::Texture, texturePair.second.second);

					specification.textures.erase(texturePair.first);
				}
				ImGui::SameLine();

				bool textChanged = false;
				if (ImGui::TreeNode(texTreeId.c_str()))
				{
					textChanged = UI::InputTextOnSameline(textureSpec.name, "##texName" + std::to_string(texId));
					int currBindSlot = textureSpec.bindSlot;
					if (ImGui::InputInt("Bind slot", &currBindSlot))
					{
						textureSpec.bindSlot = currBindSlot;
					}

					ImGui::TreePop();
				}
				else
				{
					textChanged = UI::InputTextOnSameline(textureSpec.name, "##texName" + std::to_string(texId));
				}

				if (textChanged)
				{
					SetAttributeName(textureSpec.name, texturePair.second.second);
				}

				texId++;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Framebuffers"))
		{
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

			uint32_t bufferId = 0;
			for (auto& framebufferPair : specification.framebuffers)
			{
				auto& framebufferSpec = framebufferPair.second.first;
				std::string bufferTreeId = "##buffer" + std::to_string(bufferId);

				std::string butId = "-##frameRm" + std::to_string(bufferId);
				if (ImGui::Button(butId.c_str()))
				{
					RemoveAttribute(RenderAttributeType::Framebuffer, framebufferPair.second.second);

					specification.framebuffers.erase(framebufferPair.first);
				}

				ImGui::SameLine();

				bool textChanged = false;
				if (ImGui::TreeNode(bufferTreeId.c_str()))
				{
					textChanged = UI::InputTextOnSameline(framebufferSpec.name, "##bufferName" + std::to_string(bufferId));

					if (ImGui::Button("Add##att"))
					{
						framebufferSpec.attachments.push_back(PassFramebufferAttachmentSpec());
					}

					uint32_t attId = 0;
					for (auto& attachment : framebufferSpec.attachments)
					{
						std::string butId = "-##attRm" + std::to_string(attId);
						if (ImGui::Button(butId.c_str()))
						{
							Utility::RemoveFromContainer(framebufferSpec.attachments, attachment);
						}

						ImGui::SameLine();

						std::string attachmentString = "Attachment##buffAtt" + std::to_string(attId);
						if (ImGui::TreeNode(attachmentString.c_str()))
						{
							static const char* textureTypes[] = { "Color", "Depth" };
							int currentlySelectedType = (int)attachment.type;
							std::string texTypeId = "Texure Type##" + std::to_string(bufferId);
							if (ImGui::Combo(texTypeId.c_str(), &currentlySelectedType, textureTypes, IM_ARRAYSIZE(textureTypes)))
							{
								attachment.type = (TextureType)currentlySelectedType;
							}

							int currBind = attachment.bindId;
							if (ImGui::InputInt("Bind slot", &currBind))
							{
								attachment.bindId = (uint32_t)currBind;
							}

							int currAttach = attachment.attachmentId;
							if (ImGui::InputInt("Attachment slot", &currAttach))
							{
								attachment.attachmentId = (uint32_t)currAttach;
							}

							ImGui::TreePop();
						}

						attId++;
					}

					ImGui::TreePop();
				}
				else
				{
					textChanged = UI::InputTextOnSameline(framebufferSpec.name, "##bufferName" + std::to_string(bufferId));
				}

				if (textChanged)
				{
					SetAttributeName(framebufferSpec.name, framebufferPair.second.second);
				}

				bufferId++;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Framebuffer commands"))
		{
			if (ImGui::Button("Add"))
			{
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
				GraphUUID cmdId = GraphUUID();
				specification.framebufferCommands.emplace(cmdId, std::make_pair(PassFramebufferCommandSpecification(renderPass->GetSpecification().TargetFramebuffer), input->id));

				input->data = cmdId;
				input->pNode = this;
				input->name = "FramebufferCmd" + std::to_string(specification.framebuffers.size() - 1);
				input->type = RenderAttributeType::Framebuffer;

				inputs.push_back(input);
			}

			for (auto& commandPair : specification.framebufferCommands)
			{
				auto& commandSpec = commandPair.second.first;


			}

			ImGui::TreePop();
		}

		DrawAttributes();

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
		auto& specification = renderPass->GetSpecification();

		specification.Name = node["name"].as<std::string>();
		specification.clearType = (ClearType)node["clearType"].as<uint32_t>();
		specification.drawType = (DrawType)node["drawType"].as<uint32_t>();
		specification.cullFace = (CullFace)node["cullFace"].as<uint32_t>();

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
			Renderer3D::GetSettings().UseViewportSize.push_back(specification.TargetFramebuffer);
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
				for (size_t j = 0; j < links.size(); j++)
				{
					if (links[j]->pInput->id == inputs[i]->id)
					{
						links[j]->markedForDelete = true;
						break;
					}
				}

				inputs.erase(inputs.begin() + i);

				int newIndex = 0;
				for (int i = 0; i < inputs.size(); i++)
				{
					if (inputs[i]->type == RenderAttributeType::Texture)
					{
						inputs[i]->data = newIndex;
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

	bool RenderNodePass::IsAttributeLinked(Ref<RenderAttribute> attr)
	{
		for (const auto& link : links)
		{
			if (link->pInput->id == attr->id)
			{
				return true;
			}

			if (link->pOutput->id == attr->id)
			{
				return true;
			}
		}

		return false;
	}
}