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
		m_ShaderStrings.clear();
		m_ShaderStrings.push_back("None");
		for (const auto& shader : ShaderLibrary::GetShaders())
		{
			if (shader->GetSpecification().type & ShaderType::ComputeShader)
			{
				m_ShaderStrings.push_back(shader->GetName().c_str());
			}
		}

		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(213, 234, 42, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(231, 244, 123, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(231, 244, 123, 255));

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
		if (m_ComputeShader)
		{
			auto it = std::find(m_ShaderStrings.begin(), m_ShaderStrings.end(), m_ComputeShader->GetName().c_str());
			currentlySelectedShader = (int)std::distance(m_ShaderStrings.begin(), it);
		}

		std::string shaderId = "Compute Shader##" + std::to_string(id);
		if (ImGui::Combo(shaderId.c_str(), &currentlySelectedShader, m_ShaderStrings.data(), (int)m_ShaderStrings.size()))
		{
			if (currentlySelectedShader == 0)
			{
				m_ComputeShader = nullptr;
			}
			else
			{
				m_ComputeShader = ShaderLibrary::GetShader(m_ShaderStrings[currentlySelectedShader]);
			}
		}

		DrawAttributes();

		ImGui::PopItemWidth();

		ImNodes::EndNode();

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeCompute::Activate(std::any value)
	{
		glm::vec2 bufferSize = Renderer::s_pSceneData->bufferSize;
		m_WorkGroupX = ((uint32_t)bufferSize.x + ((uint32_t)bufferSize.x % 16)) / 16;
		m_WorkGroupY = ((uint32_t)bufferSize.y + ((uint32_t)bufferSize.y % 16)) / 16;

		if (m_ComputeShader)
		{
			m_ComputeShader->Bind();
			m_ComputeShader->UploadInt("u_DepthMap", 0);
			m_ComputeShader->UploadFloat2("u_BufferSize", Renderer::s_pSceneData->bufferSize);
			m_ComputeShader->UploadInt("u_LightCount", Renderer::s_pSceneData->pointLightCount);

			if (framebuffer)
			{
				framebuffer->BindDepthAttachment(0);
			}

			glDispatchCompute(m_WorkGroupX, m_WorkGroupY, 1);

			m_ComputeShader->Unbind();
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
		LP_SERIALIZE_PROPERTY(shader, m_ComputeShader->GetName(), out);

		SerializeAttributes(out);
	}

	void RenderNodeCompute::Deserialize(YAML::Node& node)
	{
		std::string shader = node["shader"].as<std::string>();
		if (!shader.empty())
		{
			m_ComputeShader = ShaderLibrary::GetShader(shader);
		}

		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}
}