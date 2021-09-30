#include "lppch.h"
#include "RenderNodePass.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "RenderNodeEnd.h"
#include "Lamp/Utility/StandardUtilities.h"

#include <imnodes.h>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Lamp
{
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

		Ref<RenderInputAttribute> targetBuffer = CreateRef<RenderInputAttribute>();
		targetBuffer->name = "Target framebuffer";
		targetBuffer->pNode = this;
		targetBuffer->type = RenderAttributeType::Framebuffer;

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

			if (link->pInput->type == RenderAttributeType::Framebuffer)
			{
				if (RenderNodePass* passNode = dynamic_cast<RenderNodePass*>(link->pInput->pNode))
				{
					
				}
			}
		}
	}

	void RenderNodePass::DrawNode()
	{
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

		if (ImGui::TreeNode("Static Uniforms"))
		{
			if (ImGui::Button("Create"))
			{
				specification.staticUniforms.push_back(std::make_tuple("Uniform", UniformType::Int, 0));
			}

			for (int i = 0; i < specification.staticUniforms.size(); i++)
			{
				auto& [name, type, value] = specification.staticUniforms[i];

				std::string butId = "-##statRm" + std::to_string(i);
				if (ImGui::Button(butId.c_str()))
				{
					std::string sName = name;
					Utility::RemoveFromContainerIf(specification.staticUniforms, [&sName](const std::tuple<std::string, UniformType, std::any>& t) { return std::get<0>(t) == sName; });
				}

				ImGui::SameLine();

				ImGui::PushItemWidth(100.f);

				std::string uNameId = "##uniformName" + std::to_string(i);
				ImGui::InputText(uNameId.c_str(), &name);
				ImGui::SameLine();

				static const char* uTypes[] = { "Int", "Float", "Float2", "Float3", "Float4", "Mat3", "Mat4", "Sampler2D", "SamplerCube", "RenderData" };

				std::string uTypeId = "##uniformType" + std::to_string(i);
				int currentlySelectedType = (int)type;
				if (ImGui::Combo(uTypeId.c_str(), &currentlySelectedType, uTypes, 10))
				{
					type = (UniformType)currentlySelectedType;
					value = Utils::GetResetValue(type);
				}

				ImGui::PopItemWidth();

				ImGui::SameLine();

				ImGui::PushItemWidth(100.f);
				if (value.has_value())
				{
					if (type != UniformType::RenderData)
					{
						std::string inId = "##value" + std::to_string(i);
						switch (type)
						{
							case UniformType::Int:
							{
								int data = std::any_cast<int>(value);

								if (ImGui::InputInt(inId.c_str(), &data))
								{
									value = data;
								}
								break;
							}

							case UniformType::Float:
							{
								float data = std::any_cast<float>(value);
								if (ImGui::InputFloat(inId.c_str(), &data))
								{
									value = data;
								}
								break;
							}

							case UniformType::Float2:
							{
								glm::vec2 data = std::any_cast<glm::vec2>(value);
								if (ImGui::InputFloat2(inId.c_str(), glm::value_ptr(data)))
								{
									value = data;
								}
								break;
							}

							case UniformType::Float3:
							{
								glm::vec3 data = std::any_cast<glm::vec3>(value);
								if (ImGui::InputFloat3(inId.c_str(), glm::value_ptr(data)))
								{
									value = data;
								}
								break;
							}

							case UniformType::Float4:
							{
								glm::vec4 data = std::any_cast<glm::vec4>(value);
								if (ImGui::InputFloat4(inId.c_str(), glm::value_ptr(data)))
								{
									value = data;
								}
								break;
							}

							case UniformType::Mat3:
							{
								glm::mat3 data = std::any_cast<glm::mat3>(value);

								break;
							}

							case UniformType::Mat4:
							{
								glm::mat4 data = std::any_cast<glm::mat4>(value);

								break;
							}

							case UniformType::Sampler2D:
							{
								int data = std::any_cast<int>(value);
								if (ImGui::InputInt(inId.c_str(), &data))
								{
									value = data;
								}
								break;
							}

							case UniformType::SamplerCube:
							{
								int data = std::any_cast<int>(value);
								if (ImGui::InputInt(inId.c_str(), &data))
								{
									value = data;
								}
								break;
							}

						}
					}
					else
					{
						static const char* dTypes[] = { "Transform", "Data", "Material", "Id" };
						std::string dTypeId = "##dataType" + std::to_string(i);
						int currentlySelectedData = (int)std::any_cast<RenderData>(value);
						if (ImGui::Combo(dTypeId.c_str(), &currentlySelectedData, dTypes, 4))
						{
							value = (RenderData)currentlySelectedData;
						}
					}
				}

				ImGui::PopItemWidth();
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Dynamic Uniforms"))
		{
			if (ImGui::Button("Create"))
			{
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
				specification.dynamicUniforms.push_back(std::make_tuple("Uniform", UniformType::Int, nullptr, input->id));

				input->pNode = this;
				input->name = "Uniform" + std::to_string(specification.dynamicUniforms.size() - 1);
				input->type = RenderAttributeType::DynamicUniform;
				input->data = (uint32_t)(specification.dynamicUniforms.size() - 1);

				inputs.push_back(input);
			}

			for (int i = 0; i < specification.dynamicUniforms.size(); i++)
			{
				auto& [name, type, data, attrId] = specification.dynamicUniforms[i];

				std::string butId = "-##dynRm" + std::to_string(i);
				if (ImGui::Button(butId.c_str()))
				{
					RemoveAttribute(RenderAttributeType::DynamicUniform, attrId);

					std::string sName = name;
					Utility::RemoveFromContainerIf(specification.dynamicUniforms, [&sName](const std::tuple<std::string, UniformType, std::any, GraphUUID>& t) { return std::get<0>(t) == sName; });
				}

				ImGui::SameLine();

				ImGui::PushItemWidth(100.f);
				std::string nameId = "##dynUniformName" + std::to_string(i);
				ImGui::InputText(nameId.c_str(), &name);

				ImGui::PopItemWidth();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Textures"))
		{
			if (ImGui::Button("Add"))
			{
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
				specification.textures.push_back(std::make_tuple(nullptr, 0, input->id));

				input->data = (uint32_t)(specification.textures.size() - 1);
				input->pNode = this;
				input->name = "Texture" + std::to_string(specification.textures.size() - 1);
				input->type = RenderAttributeType::Texture;

				inputs.push_back(input);
			}

			uint32_t texId = 0;
			for (auto& pair : specification.textures)
			{
				auto& [texture, bindId, attrId] = pair;

				std::string texTreeId = "Texture##tex" + std::to_string(texId);

				std::string butId = "-##texRm" + std::to_string(texId);
				if (ImGui::Button(butId.c_str()))
				{
					RemoveAttribute(RenderAttributeType::Texture, attrId);

					Utility::RemoveFromContainer(specification.textures, pair);
				}
				ImGui::SameLine();
				
				if (ImGui::TreeNode(texTreeId.c_str()))
				{
					int currBindSlot = bindId;
					if (ImGui::InputInt("Bind slot", &currBindSlot))
					{
						bindId = currBindSlot;
					}

					ImGui::TreePop();
				}

				texId++;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Framebuffers"))
		{
			if (ImGui::Button("Add"))
			{
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
				specification.framebuffers.push_back({ nullptr, TextureType::Color, 0, 0, input->id });

				input->pData = &specification.framebuffers[specification.framebuffers.size() - 1];
				input->pNode = this;
				input->name = "Framebuffer" + std::to_string(specification.framebuffers.size() - 1);
				input->type = RenderAttributeType::Framebuffer;

				inputs.push_back(input);
			}

			uint32_t bufferId = 0;
			for (auto& frameBuf : specification.framebuffers)
			{
				auto& [buffer, type, bind, attach, attrId] = frameBuf;
				std::string bufferTreeId = "Framebuffer##buffer" + std::to_string(bufferId);

				std::string butId = "-##frameRm" + std::to_string(bufferId);
				if (ImGui::Button(butId.c_str()))
				{
					RemoveAttribute(RenderAttributeType::Framebuffer, attrId);

					Utility::RemoveFromContainer(specification.framebuffers, frameBuf);
				}

				ImGui::SameLine();

				if (ImGui::TreeNode(bufferTreeId.c_str()))
				{
					static const char* textureTypes[] = { "Color", "Depth" };
					int currentlySelectedType = (int)type;
					std::string texTypeId = "Texure Type##" + std::to_string(bufferId);
					if (ImGui::Combo(texTypeId.c_str(), &currentlySelectedType, textureTypes, IM_ARRAYSIZE(textureTypes)))
					{
						type = (TextureType)currentlySelectedType;
					}

					int currBind = bind;
					if (ImGui::InputInt("Bind slot", &currBind))
					{
						bind = (uint32_t)currBind;
					}

					int currAttach = attach;
					if (ImGui::InputInt("Attachment slot", &currAttach))
					{
						attach = (uint32_t)currAttach;
					}

					ImGui::TreePop();
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
				specification.framebufferCommands.push_back(std::make_tuple(renderPass->GetSpecification().TargetFramebuffer, nullptr, FramebufferCommand::Copy, input->id));

				input->data = (uint32_t)(specification.framebufferCommands.size() - 1);
				input->pNode = this;
				input->name = "FramebufferCmd" + std::to_string(specification.framebuffers.size() - 1);
				input->type = RenderAttributeType::Framebuffer;

				inputs.push_back(input);
			}

			for (auto& [main, secondary, command, attrId] : specification.framebufferCommands)
			{

			}

			ImGui::TreePop();
		}

		DrawAttributes();

		ImGui::PopItemWidth();

		ImNodes::EndNode();
	}

	void RenderNodePass::Activate(std::any value)
	{
		LP_PROFILE_FUNCTION();

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

			out << YAML::Key << "attachments" << YAML::Value;
			out << YAML::BeginMap;
			{
				uint32_t attCount = 0;
				for (auto& att : targetBuffSpec.Attachments.Attachments)
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
		}
		out << YAML::EndMap; //target framebuffer

		out << YAML::Key << "staticUniforms" << YAML::Value;
		out << YAML::BeginMap;
		{
			uint32_t statUniformCount = 0;
			for (const auto& [uName, uType, uData] : specification.staticUniforms)
			{
				out << YAML::Key << "staticUniform" + std::to_string(statUniformCount) << YAML::Value;
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(name, uName, out);
				LP_SERIALIZE_PROPERTY(type, (uint32_t)uType, out);

				switch (uType)
				{
					case UniformType::Int:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<int>(uData), out);
						break;
					}

					case UniformType::Float:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<float>(uData), out);
						break;
					}

					case UniformType::Float2:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec2>(uData), out);
						break;
					}

					case UniformType::Float3:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec3>(uData), out);
						break;
					}

					case UniformType::Float4:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec4>(uData), out);
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
						LP_SERIALIZE_PROPERTY(data, std::any_cast<int>(uData), out);
						break;
					}

					case UniformType::SamplerCube:
					{
						LP_SERIALIZE_PROPERTY(data, std::any_cast<int>(uData), out);
						break;
					}

					case UniformType::RenderData:
					{
						LP_SERIALIZE_PROPERTY(data, (uint32_t)std::any_cast<RenderData>(uData), out);
						break;
					}
				}

				out << YAML::EndMap;
				statUniformCount++;
			}
		}
		out << YAML::EndMap; //static uniforms

		out << YAML::Key << "dynamicUniforms" << YAML::Value;
		out << YAML::BeginMap;
		{
			uint32_t dynUniformCount = 0;
			for (const auto& [uName, uType, uData, attrId] : specification.dynamicUniforms)
			{
				out << YAML::Key << "dynamicUniform" + std::to_string(dynUniformCount) << YAML::Value;
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(name, uName, out);
				LP_SERIALIZE_PROPERTY(type, (uint32_t)uType, out);
				LP_SERIALIZE_PROPERTY(attrId, attrId, out);

				out << YAML::EndMap;
				dynUniformCount++;
			}
		}
		out << YAML::EndMap; //dynamic uniforms

		out << YAML::Key << "textures" << YAML::Value;
		out << YAML::BeginMap;
		{
			uint32_t texCount = 0;
			for (const auto& [uTexture, uBindSlot, attrId] : specification.textures)
			{
				out << YAML::Key << "texture" + std::to_string(texCount) << YAML::Value;
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(bindSlot, uBindSlot, out);
				LP_SERIALIZE_PROPERTY(attrId, attrId, out);

				out << YAML::EndMap;
				texCount++;
			}
		}
		out << YAML::EndMap; //textures

		out << YAML::Key << "framebuffers" << YAML::Value;
		out << YAML::BeginMap;
		{
			uint32_t bufferCount = 0;
			for (const auto& [buffer, texType, bindSlot, attachId, attrId] : specification.framebuffers)
			{
				out << YAML::Key << "framebuffer" + std::to_string(bufferCount) << YAML::Value;
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(textureType, (uint32_t)texType, out);
				LP_SERIALIZE_PROPERTY(bindSlot, bindSlot, out);
				LP_SERIALIZE_PROPERTY(attachmentId, attachId, out);
				LP_SERIALIZE_PROPERTY(attrId, attrId, out);

				out << YAML::EndMap;
				bufferCount++;
			}
		}
		out << YAML::EndMap; //framebuffers

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
		uint32_t attachmentCount = 0;
		while (YAML::Node attachmentNode = attachmentsNode["attachment" + std::to_string(attachmentCount)])
		{
			FramebufferTextureSpecification att;

			LP_DESERIALIZE_PROPERTY(borderColor, att.BorderColor, attachmentNode, glm::vec4(0.f));
			LP_DESERIALIZE_PROPERTY(multisampled, att.MultiSampled, attachmentNode, false);

			att.TextureFormat = (FramebufferTextureFormat)attachmentNode["format"].as<uint32_t>();
			att.TextureFiltering = (FramebufferTexureFiltering)attachmentNode["filtering"].as<uint32_t>();
			att.TextureWrap = (FramebufferTextureWrap)attachmentNode["wrap"].as<uint32_t>();

			targetBufferSpec.Attachments.Attachments.push_back(att);
			attachmentCount++;
		}

		specification.TargetFramebuffer = Framebuffer::Create(specification.TargetFramebuffer->GetSpecification());

		if (m_UseViewportSize)
		{
			Renderer3D::GetSettings().UseViewportSize.push_back(specification.TargetFramebuffer);
		}

		//static uniforms
		YAML::Node staticUniformsNode = node["staticUniforms"];
		uint32_t statUniformCount = 0;

		while (YAML::Node uniformNode = staticUniformsNode["staticUniform" + std::to_string(statUniformCount)])
		{
			std::string uName = uniformNode["name"].as<std::string>();
			UniformType uType = (UniformType)uniformNode["type"].as<uint32_t>();
			std::any uData;

			switch (uType)
			{
				case Lamp::UniformType::Int:
					LP_DESERIALIZE_PROPERTY(data, uData, uniformNode, 0);
					break;

				case Lamp::UniformType::Float:
					LP_DESERIALIZE_PROPERTY(data, uData, uniformNode, 0.f);
					break;

				case Lamp::UniformType::Float2:
					LP_DESERIALIZE_PROPERTY(data, uData, uniformNode, glm::vec2(0.f));
					break;

				case Lamp::UniformType::Float3:
					LP_DESERIALIZE_PROPERTY(data, uData, uniformNode, glm::vec3(0.f));
					break;

				case Lamp::UniformType::Float4:
					LP_DESERIALIZE_PROPERTY(data, uData, uniformNode, glm::vec4(0.f));
					break;

				case Lamp::UniformType::Mat3:
					break;

				case Lamp::UniformType::Mat4:
					break;

				case Lamp::UniformType::Sampler2D:
					LP_DESERIALIZE_PROPERTY(data, uData, uniformNode, 0);
					break;

				case Lamp::UniformType::SamplerCube:
					LP_DESERIALIZE_PROPERTY(data, uData, uniformNode, 0);
					break;

				case Lamp::UniformType::RenderData:
				{
					uint32_t data;
					LP_DESERIALIZE_PROPERTY(data, data, uniformNode, 0);
					uData = (RenderData)data;
					break;
				}

				default:
					break;
			}

			specification.staticUniforms.push_back(std::make_tuple(uName, uType, uData));
			statUniformCount++;
		}

		//dynamic uniforms
		YAML::Node dynamicUniformsNode = node["dynamicUniforms"];
		uint32_t dynUniformCount = 0;

		while (YAML::Node uniformNode = dynamicUniformsNode["dynamicUniform" + std::to_string(dynUniformCount)])
		{
			std::string uName = uniformNode["name"].as<std::string>();
			UniformType uType = (UniformType)uniformNode["type"].as<uint32_t>();
			
			GraphUUID attrId;
			LP_DESERIALIZE_PROPERTY(attrId, attrId, uniformNode, 0);

			specification.dynamicUniforms.push_back(std::make_tuple(uName, uType, nullptr, attrId));
			dynUniformCount++;
		}

		//textures
		YAML::Node texturesNode = node["textures"];
		uint32_t texCount = 0;

		while (YAML::Node texNode = texturesNode["texture" + std::to_string(texCount)])
		{
			uint32_t bindSlot;
			GraphUUID attrId;
			LP_DESERIALIZE_PROPERTY(bindSlot, bindSlot, texNode, 0);
			LP_DESERIALIZE_PROPERTY(attrId, attrId, texNode, 0);

			specification.textures.push_back(std::make_tuple(nullptr, bindSlot, attrId));
			texCount++;
		}

		//framebuffers
		YAML::Node framebuffersNode = node["framebuffers"];
		uint32_t bufferCount = 0;

		while (YAML::Node bufferNode = framebuffersNode["framebuffer" + std::to_string(bufferCount)])
		{
			TextureType type = (TextureType)bufferNode["textureType"].as<uint32_t>();
			uint32_t bindSlot;
			uint32_t attachId;
			GraphUUID attrId;

			LP_DESERIALIZE_PROPERTY(bindSlot, bindSlot, bufferNode, 0);
			LP_DESERIALIZE_PROPERTY(attachmentId, attachId, bufferNode, 0);
			LP_DESERIALIZE_PROPERTY(attrId, attrId, bufferNode, 0);

			specification.framebuffers.push_back(std::make_tuple(nullptr, type, bindSlot, attachId, attrId));
			bufferCount++;
		}

		//attributes
		outputs.clear();
		inputs.clear();
		uint32_t attributeCount = 0;
		uint32_t uniformIndex = 0;
		uint32_t bufferIndex = 0;
		uint32_t textureIndex = 0;

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

			attributeCount++;
		}
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

}