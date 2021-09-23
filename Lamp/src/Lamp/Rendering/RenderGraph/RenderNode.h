#pragma once

#include <string>

#include "Lamp/Rendering/RenderPass.h"

namespace Lamp
{
	struct RenderNode;
	struct RenderLink;

	struct RenderAttribute
	{
		virtual ~RenderAttribute() {}

		std::vector<Ref<RenderLink>> links;
		RenderNode* pNode;
		uint32_t id;
		std::string name;
	};

	struct RenderOutputAttribute : public RenderAttribute
	{};

	struct RenderInputAttribute : public RenderAttribute
	{
		std::any data;
	};

	struct RenderLink
	{
		RenderOutputAttribute* pOutput;
		RenderInputAttribute* pInput;
		uint32_t id;
	};

	struct RenderNode
	{
		RenderNode()
			: currId(UINT32_MAX - 1000)
		{}
		virtual ~RenderNode() {}

		glm::vec2 position;
	
		std::vector<Ref<RenderLink>> links;
		std::vector<RenderOutputAttribute> outputs;
		std::vector<RenderInputAttribute> inputs;

		uint32_t currId;
		uint32_t id;

		virtual void Start() = 0;
		virtual void DrawNode() = 0;
		virtual void Activate(std::any value) = 0;
	};

	struct RenderNodePass : public RenderNode
	{
		RenderNodePass()
		{
			renderPass = CreateRef<RenderPass>();
		}

		virtual ~RenderNodePass() override {}

		std::string name;
		Ref<RenderPass> renderPass;

		virtual void Start() override {}
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override; 
	
	private:
		std::vector<const char*> m_Shaders;
	};

	struct RenderNodeFramebuffer : public RenderNode
	{
		RenderNodeFramebuffer()
		{
			framebuffer = Framebuffer::Create(FramebufferSpecification());
			
			RenderOutputAttribute output;
			output.pNode = this;
			output.name = "Output";
			output.id = ++currId;

			outputs.push_back(output);
		}

		virtual ~RenderNodeFramebuffer() override {}

		Ref<Framebuffer> framebuffer;

		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
	};
}