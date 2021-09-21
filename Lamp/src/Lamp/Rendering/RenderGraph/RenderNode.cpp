#include "lppch.h"
#include "RenderNode.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imnodes.h>

#include "Lamp/Rendering/Shader/ShaderLibrary.h"

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

				switch (type)
				{
					case UniformType::Int:
					{
						int data = std::any_cast<int>(value);
						if (ImGui::InputInt("##int", &data))
						{
							value = data;
						}
						break;
					}

					case UniformType::Float:
					{
						float data = std::any_cast<float>(value);
						if (ImGui::InputFloat("##float", &data))
						{
							value = data;
						}
						break;
					}

					case UniformType::Float2:
					{
						glm::vec2 data = std::any_cast<glm::vec2>(value);
						if (ImGui::InputFloat2("##float2", glm::value_ptr(data)))
						{
							value = data;
						}
						break;
					}

					case UniformType::Float3:
					{
						glm::vec3 data = std::any_cast<glm::vec3>(value);
						if (ImGui::InputFloat3("##float3", glm::value_ptr(data)))
						{
							value = data;
						}
						break;
					}

					case UniformType::Float4:
					{
						glm::vec4 data = std::any_cast<glm::vec4>(value);
						if (ImGui::InputFloat4("##float4", glm::value_ptr(data)))
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
						if (ImGui::InputInt("##int", &data))
						{
							value = data;
						}
						break;
					}

					case UniformType::SamplerCube:
					{
						int data = std::any_cast<int>(value);
						if (ImGui::InputInt("##int", &data))
						{
							value = data;
						}
						break;
					}

				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Framebuffers"))
		{
			if (ImGui::Button("Add"))
			{
				specification.framebuffers.push_back({ nullptr, TextureType::Color, 0, 0 });
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

		ImGui::PopItemWidth();

		ImNodes::EndNode();
	}

	void RenderNodePass::Activate(std::any value)
	{
		Ref<CameraBase> camera = std::any_cast<Ref<CameraBase>>(value);
		renderPass->Render(camera);
	}

	void RenderNodeFramebuffer::DrawNode()
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 28, 17, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(179, 53, 41, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(179, 53, 41, 255));

		ImNodes::BeginNode(id);

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
		ImGui::InputFloat4("Clear Color", glm::value_ptr(specification.ClearColor));
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
					ImGui::InputFloat4("Border Color", glm::value_ptr(att.BorderColor));

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
						framebuffer = Framebuffer::Create(framebuffer->GetSpecification());
					}


					ImGui::TreePop();
				}
				attIndex++;
			}
			ImGui::PopItemWidth();

			ImGui::TreePop();
		}

		for (int i = 0; i < outputAttributes.size(); i++)
		{
			ImNodes::BeginOutputAttribute(id + i);

			ImGui::Text("Output");

			ImNodes::EndOutputAttribute();
		}

		ImNodes::EndNode();

		ImGui::PopItemWidth();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}


	void RenderNodeFramebuffer::Activate(std::any value)
	{
	}
}