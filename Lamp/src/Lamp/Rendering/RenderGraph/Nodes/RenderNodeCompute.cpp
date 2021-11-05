#include "lppch.h"
#include "RenderNodeCompute.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Utility/UIUtility.h"
#include "Lamp/Rendering/RenderGraph/RenderGraphUtils.h"

#include <imnodes.h>

namespace Lamp
{
	namespace Utils
	{
		static std::any GetResetValue(UniformType type)
		{
			switch (type)
			{
				case UniformType::Int: return 0;
				case UniformType::Float: return 0.f;
				case UniformType::Float2: return glm::vec2(0.f);
				case UniformType::Float3: return glm::vec3(0.f);
				case UniformType::Float4: return glm::vec4(0.f);
				case UniformType::Mat3: return glm::mat3(1.f);
				case UniformType::Mat4: return glm::mat4(1.f);
				case UniformType::Sampler2D: return 0;
				case UniformType::SamplerCube: return 0;
				case UniformType::RenderData: return RenderData::Transform;
				default:
					return -1;
			}
		}
	}

	void RenderNodeCompute::Initialize()
	{
		m_runAttribute = CreateRef<RenderInputAttribute>();
		m_runAttribute->name = "Run";
		m_runAttribute->pNode = this;
		m_runAttribute->type = RenderAttributeType::Pass;

		Ref<RenderInputAttribute> depthMap = CreateRef<RenderInputAttribute>();
		depthMap->name = "Depth";
		depthMap->pNode = this;
		depthMap->type = RenderAttributeType::Framebuffer;

		Ref<RenderOutputAttribute> activatedAttr = CreateRef<RenderOutputAttribute>();
		activatedAttr->name = "Finished";
		activatedAttr->pNode = this;
		activatedAttr->type = RenderAttributeType::Pass;

		inputs.push_back(m_runAttribute);
		inputs.push_back(depthMap);
		outputs.push_back(activatedAttr);
	}

	void RenderNodeCompute::DrawNode()
	{
		m_shaderStrings.clear();
		m_shaderStrings.push_back("None");
		for (const auto& shader : ShaderLibrary::GetShaders())
		{
			if (shader->GetSpecification().type & ShaderType::ComputeShader)
			{
				m_shaderStrings.push_back(shader->GetName().c_str());
			}
		}

		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(232, 147, 74, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(255, 190, 94, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(255, 190, 94, 255));

		ImNodes::BeginNode(id);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("Compute shader");
		ImNodes::EndNodeTitleBar();

		ImGui::PushItemWidth(150.f);

		int currentlySelectedShader = 0;
		if (m_computeShader)
		{
			auto it = std::find(m_shaderStrings.begin(), m_shaderStrings.end(), m_computeShader->GetName().c_str());
			currentlySelectedShader = (int)std::distance(m_shaderStrings.begin(), it);
		}

		ImGui::TextUnformatted("Compute shader");

		ImGui::SameLine();

		std::string shaderId = "##" + std::to_string(id);
		if (ImGui::Combo(shaderId.c_str(), &currentlySelectedShader, m_shaderStrings.data(), (int)m_shaderStrings.size()))
		{
			if (currentlySelectedShader == 0)
			{
				m_computeShader = nullptr;
			}
			else
			{
				m_computeShader = ShaderLibrary::GetShader(m_shaderStrings[currentlySelectedShader]);
				SetupUniforms();
			}
		}

		if (UI::TreeNodeFramed("Uniforms", ImNodes::GetNodeDimensions(id).x - 20.f))
		{
			if (m_uniforms.empty())
			{
				ImGui::TextColored({ 0.874, 0.165, 0.164, 1.f }, "There are no uniforms to show!");
			}
			else
			{
				DrawUniforms();
			}

			UI::TreeNodePop();
		}
		else
		{
			for (auto& uniformPair : m_uniforms)
			{
				FindAttributeByID(uniformPair.second.second)->shouldDraw = true;
			}
		}

		DrawAttributes({ m_runAttribute }, outputs);

		for (const auto& input : inputs)
		{
			if (IsAttributeLinked(input) && input->shouldDraw)
			{
				unsigned int pinColor = Utils::GetTypeColor(input->type);
				unsigned int pinHoverColor = Utils::GetTypeHoverColor(input->type);

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

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeCompute::Activate(std::any value)
	{
		glm::vec2 bufferSize = Renderer::s_pSceneData->bufferSize;
		m_workGroupX = ((uint32_t)bufferSize.x + ((uint32_t)bufferSize.x % 16)) / 16;
		m_workGroupY = ((uint32_t)bufferSize.y + ((uint32_t)bufferSize.y % 16)) / 16;

		if (m_computeShader)
		{
			for (const auto& uniformPairs : m_uniforms)
			{
				const auto& uniform = uniformPairs.second.first;
				switch (uniform.type)
				{
					case UniformType::Int: m_computeShader->UploadInt(uniform.name, std::any_cast<int>(uniform.data)); break;
					case UniformType::Float: m_computeShader->UploadFloat(uniform.name, std::any_cast<float>(uniform.data)); break;
					case UniformType::Float2: m_computeShader->UploadFloat2(uniform.name, std::any_cast<glm::vec2>(uniform.data)); break;
					case UniformType::Float3: m_computeShader->UploadFloat3(uniform.name, std::any_cast<glm::vec3>(uniform.data)); break;
					case UniformType::Float4: m_computeShader->UploadFloat4(uniform.name, std::any_cast<glm::vec4>(uniform.data)); break;
					case UniformType::Mat3: m_computeShader->UploadMat3(uniform.name, std::any_cast<glm::mat3>(uniform.data)); break;
					case UniformType::Mat4: m_computeShader->UploadMat4(uniform.name, std::any_cast<glm::mat4>(uniform.data)); break;
					case UniformType::Sampler2D: m_computeShader->UploadInt(uniform.name, std::any_cast<int>(uniform.data)); break;
					case UniformType::SamplerCube: m_computeShader->UploadInt(uniform.name, std::any_cast<int>(uniform.data)); break;
					case UniformType::RenderData: break;

					default:
						break;
				}
			}

			if (framebuffer)
			{
				framebuffer->BindDepthAttachment(0);
			}

			glDispatchCompute(m_workGroupX, m_workGroupY, 1);

			m_computeShader->Unbind();
		}

		for (const auto& link : links)
		{
			if (link->pInput->pNode->id == id)
			{
				continue;
			}

			link->pInput->pNode->Activate(value);
		}
	}

	void RenderNodeCompute::Serialize(YAML::Emitter& out)
	{
		LP_SERIALIZE_PROPERTY(shader, m_computeShader->GetName(), out);

		SerializeAttributes(out);
	}

	void RenderNodeCompute::Deserialize(YAML::Node& node)
	{
		std::string shader = node["shader"].as<std::string>();
		if (!shader.empty())
		{
			m_computeShader = ShaderLibrary::GetShader(shader);
		}

		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}

	void RenderNodeCompute::SetupUniforms()
	{
		m_uniforms.clear();

		for (const auto& uniform : m_computeShader->GetSpecification().uniforms)
		{
			PassUniformSpecification spec{ uniform.name, uniform.type, uniform.data, uniform.id };

			GraphUUID uniformId = GraphUUID();
			Ref<RenderInputAttribute> input = CreateRef<RenderInputAttribute>();

			m_uniforms.emplace(uniformId, std::make_pair<>(spec, input->id));

			input->pNode = this;
			input->name = uniform.name;
			input->type = RenderAttributeType::DynamicUniform;
			input->data = uniformId;

			inputs.push_back(input);
		}
	}

	void RenderNodeCompute::DrawUniforms()
	{
		for (auto& uniformPair : m_uniforms)
		{
			auto& uniform = uniformPair.second.first;

			ImGui::PushID(uniform.name.c_str());
			uint32_t stackId = 0;

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

			std::string comboId = "##" + std::to_string(stackId++);
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
					std::string inputId = "##" + std::to_string(stackId++);
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
					std::string dTypeId = "##" + std::to_string(stackId++);
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
		}
	}
}