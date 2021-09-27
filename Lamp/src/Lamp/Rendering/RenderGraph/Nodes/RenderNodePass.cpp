#include "lppch.h"
#include "RenderNodePass.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

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
				default:
					return -1;
			}
		}
	}

	void RenderNodePass::Initialize()
	{
		renderPass = CreateRef<RenderPass>();

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->id = ++currId;
		output->name = "Framebuffer";
		output->pNode = this;
		output->type = RenderAttributeType::Framebuffer;

		outputs.push_back(output);
	}

	void RenderNodePass::Start()
	{
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
				static const char* clearTypes[4] = { "None", "Color", "Depth", "ColorDepth" };
				int currentlySelectedClearType = (int)specification.clearType;
				std::string clearTypeId = "Clear Type##node" + nodeId;

				if (ImGui::Combo(clearTypeId.c_str(), &currentlySelectedClearType, clearTypes, 4))
				{
					specification.clearType = (ClearType)currentlySelectedClearType;
				}
			}

			//DrawType
			{
				static const char* drawTypes[3] = { "All", "Quad", "Line" };
				int currentlySelectedDrawType = (int)specification.drawType;
				std::string drawTypeId = "Draw Type##node" + nodeId;

				if (ImGui::Combo(drawTypeId.c_str(), &currentlySelectedDrawType, drawTypes, 3))
				{
					specification.drawType = (DrawType)currentlySelectedDrawType;
				}
			}

			//Cull face
			{
				static const char* cullFaces[2] = { "Front", "Back" };
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
					currentlySelectedShader = std::distance(m_Shaders.begin(), it);
				}
				std::string shaderId = "Render Shader##node" + nodeId;
				if (ImGui::Combo(shaderId.c_str(), &currentlySelectedShader, m_Shaders.data(), m_Shaders.size()))
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

		if (ImGui::TreeNode("Static Uniforms"))
		{
			if (ImGui::Button("Create"))
			{
				specification.staticUniforms.push_back(std::make_tuple("Uniform", UniformType::Int, 0));
			}

			for (int i = 0; i < specification.staticUniforms.size(); i++)
			{
				auto& [name, type, value] = specification.staticUniforms[i];

				ImGui::PushItemWidth(100.f);

				std::string uNameId = "##uniformName" + std::to_string(i);
				ImGui::InputText(uNameId.c_str(), &name);
				ImGui::SameLine();

				static const char* uTypes[] = { "Int", "Float", "Float2", "Float3", "Float4", "Mat3", "Mat4", "Sampler2D", "SamplerCube" };

				std::string uTypeId = "##uniformType" + std::to_string(i);
				int currentlySelectedType = (int)type;
				if (ImGui::Combo(uTypeId.c_str(), &currentlySelectedType, uTypes, 9))
				{
					type = (UniformType)currentlySelectedType;
					value = Utils::GetResetValue(type);
				}

				ImGui::PopItemWidth();

				ImGui::SameLine();

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

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Dynamic Uniforms"))
		{
			if (ImGui::Button("Create"))
			{
				specification.dynamicUniforms.push_back(std::make_tuple("Uniform", UniformType::Int, nullptr));
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();

				input->pNode = this;
				input->name = "Unifrom" + std::to_string(specification.dynamicUniforms.size() - 1);
				input->id = ++currId;
				input->type = RenderAttributeType::DynamicUniform;

				inputs.push_back(input);
			}

			for (int i = 0; i < specification.dynamicUniforms.size(); i++)
			{
				ImGui::PushItemWidth(100.f);

				auto& [name, type, data] = specification.dynamicUniforms[i];

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
				specification.textures.push_back(std::make_pair(nullptr, 0));
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
				input->data = (uint32_t)(specification.textures.size() - 1);
				input->pNode = this;
				input->name = "Texture" + std::to_string(specification.textures.size() - 1);
				input->id = ++currId;
				input->type = RenderAttributeType::Texture;

				inputs.push_back(input);
			}

			uint32_t texId = 0;
			for (auto& [texture, bindId] : specification.textures)
			{
				std::string texTreeId = "Texture##tex" + std::to_string(texId);
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
				specification.framebuffers.push_back({ nullptr, TextureType::Color, 0, 0 });
				Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();
				input->data = (uint32_t)(specification.framebuffers.size() - 1);
				input->pNode = this;
				input->name = "Framebuffer" + std::to_string(specification.framebuffers.size() - 1);
				input->id = ++currId;
				input->type = RenderAttributeType::Framebuffer;

				inputs.push_back(input);
			}

			uint32_t bufferId = 0;
			for (auto& [buffer, type, bind, attach] : specification.framebuffers)
			{
				std::string bufferTreeId = "Framebuffer##buffer" + std::to_string(bufferId);
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
				//specification.framebufferCommands.push_back(std::make_tuple(nullptr, nullptr, FramebufferCommand::Copy));

			}

			for (auto& [main, secondary, command] : specification.framebufferCommands)
			{

			}

			ImGui::TreePop();
		}

		ImGui::TextColored(ImVec4(0.38, 0.42, 1, 1), "Inputs");

		for (auto& input : inputs)
		{
			ImNodes::BeginInputAttribute(input->id);

			ImGui::Text(input->name.c_str());

			ImNodes::EndInputAttribute();
		}

		ImGui::TextColored(ImVec4(0.101, 1, 0.313, 1), "Outputs");

		for (auto& output : outputs)
		{
			ImNodes::BeginOutputAttribute(output->id);

			ImGui::Text(output->name.c_str());

			ImNodes::EndOutputAttribute();
		}

		ImGui::PopItemWidth();

		ImNodes::EndNode();
	}

	void RenderNodePass::Activate(std::any value)
	{
		Ref<CameraBase> camera = std::any_cast<Ref<CameraBase>>(value);
		renderPass->Render(camera);
	}

	void RenderNodePass::Serialize(YAML::Emitter& out)
	{
		const auto& specification = renderPass->GetSpecification();

		LP_SERIALIZE_PROPERTY(name, specification.Name, out);
		LP_SERIALIZE_PROPERTY(clearType, (uint32_t)specification.clearType, out);
		LP_SERIALIZE_PROPERTY(drawType, (uint32_t)specification.drawType, out);
		LP_SERIALIZE_PROPERTY(cullFace, (uint32_t)specification.cullFace, out);
		LP_SERIALIZE_PROPERTY(renderShader, (specification.renderShader ? specification.renderShader->GetName() : ""), out);

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
			for (const auto& [uName, uType, uData] : specification.dynamicUniforms)
			{
				out << YAML::Key << "dynamicUniform" + std::to_string(dynUniformCount) << YAML::Value;
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(name, uName, out);
				LP_SERIALIZE_PROPERTY(type, (uint32_t)uType, out);

				out << YAML::EndMap;
				dynUniformCount++;
			}
		}
		out << YAML::EndMap; //dynamic uniforms

		out << YAML::Key << "textures" << YAML::Value;
		out << YAML::BeginMap;
		{
			uint32_t texCount = 0;
			for (const auto& [uTexture, uBindSlot] : specification.textures)
			{
				out << YAML::Key << "texture" + std::to_string(texCount) << YAML::Value;
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(bindSlot, uBindSlot, out);

				out << YAML::EndMap;
				texCount++;
			}
		}
		out << YAML::EndMap; //textures

		out << YAML::Key << "framebuffers" << YAML::Value;
		out << YAML::BeginMap;
		{
			uint32_t bufferCount = 0;
			for (const auto& [buffer, texType, bindSlot, attachId] : specification.framebuffers)
			{
				out << YAML::Key << "framebuffer" + std::to_string(bufferCount) << YAML::Value;
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(textureType, (uint32_t)texType, out);
				LP_SERIALIZE_PROPERTY(bindSlot, bindSlot, out);
				LP_SERIALIZE_PROPERTY(attachmentId, attachId, out);

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

			specification.dynamicUniforms.push_back(std::make_tuple(uName, uType, nullptr));
			dynUniformCount++;
		}

		//textures
		YAML::Node texturesNode = node["textures"];
		uint32_t texCount = 0;

		while (YAML::Node texNode = texturesNode["texture" + std::to_string(texCount)])
		{
			uint32_t bindSlot;
			LP_DESERIALIZE_PROPERTY(bindSlot, bindSlot, texNode, 0);

			specification.textures.push_back(std::make_pair(nullptr, bindSlot));
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
			
			LP_DESERIALIZE_PROPERTY(bindSlot, bindSlot, bufferNode, 0);
			LP_DESERIALIZE_PROPERTY(attachmentId, attachId, bufferNode, 0);

			specification.framebuffers.push_back(std::make_tuple(nullptr, type, bindSlot, attachId));
			bufferCount++;
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