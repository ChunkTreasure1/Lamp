#include "lppch.h"
#include "RenderNodeCompute.h"

#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Utility/PlatformUtility.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Rendering/Renderer.h"

#include <imnodes.h>

namespace Lamp
{
	void RenderNodeCompute::Initialize()
	{
		Ref<RenderInputAttribute> activateAttr = CreateRef<RenderInputAttribute>();
		activateAttr->name = "Activate";
		activateAttr->pNode = this;
		activateAttr->type = RenderAttributeType::Pass;

		Ref<RenderInputAttribute> depthMap = CreateRef<RenderInputAttribute>();
		depthMap->name = "Depth";
		depthMap->pNode = this;
		depthMap->type = RenderAttributeType::Framebuffer;

		Ref<RenderOutputAttribute> activatedAttr = CreateRef<RenderOutputAttribute>();
		activatedAttr->name = "Activated";
		activatedAttr->pNode = this;
		activatedAttr->type = RenderAttributeType::Pass;

		inputs.push_back(activateAttr);
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

		std::string shaderId = "Compute Shader##" + std::to_string(id);
		if (ImGui::Combo(shaderId.c_str(), &currentlySelectedShader, m_shaderStrings.data(), (int)m_shaderStrings.size()))
		{
			if (currentlySelectedShader == 0)
			{
				m_computeShader = nullptr;
			}
			else
			{
				m_computeShader = ShaderLibrary::GetShader(m_shaderStrings[currentlySelectedShader]);
			}
		}

		DrawAttributes(inputs, outputs);

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
			m_computeShader->Bind();
			m_computeShader->UploadInt("u_DepthMap", 0);
			m_computeShader->UploadFloat2("u_BufferSize", Renderer::s_pSceneData->bufferSize);
			m_computeShader->UploadInt("u_LightCount", Renderer::s_pSceneData->pointLightCount);

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
}