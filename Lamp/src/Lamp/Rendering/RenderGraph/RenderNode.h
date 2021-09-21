#pragma once

#include <string>

#include "Lamp/Rendering/RenderPass.h"

namespace Lamp
{
	struct RenderNode;

	struct RenderLink
	{
		uint32_t id;
	};

	struct RenderNode
	{
		glm::vec2 position;
	
		std::vector<Ref<RenderLink>> links;

		uint32_t currId;
		uint32_t id;
		virtual void DrawNode() = 0;
		virtual void Activate(std::any value) = 0;
	};

	struct RenderNodePass : public RenderNode
	{
		RenderNodePass()
		{
			renderPass = CreateRef<RenderPass>();
		}

		std::string name;
		Ref<RenderPass> renderPass;

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
		}

		Ref<Framebuffer> framebuffer;

		virtual void DrawNode() override;
		virtual void Activate(std::any value) override;
	};
}