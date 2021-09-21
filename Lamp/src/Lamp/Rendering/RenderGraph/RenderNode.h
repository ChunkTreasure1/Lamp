#pragma once

#include <string>

#include "Lamp/Rendering/RenderPass.h"

namespace Lamp
{
	struct RenderNode;

	struct RenderLink
	{
		RenderNode* inputPass;
		RenderNode* outputPass;
	
		uint32_t id;
	};

	struct RenderNode
	{
		glm::vec2 position;
		Ref<RenderLink> link;
	
		uint32_t currId;
		uint32_t id;
		virtual void DrawNode() = 0;

	protected:
		std::vector<uint32_t> outputIds;
	};

	struct RenderNodePass : public RenderNode
	{
		RenderNodePass()
		{
			renderPass = CreateRef<RenderPass>();
		}

		std::string name;
		Ref<RenderPass> renderPass;
		
		void Render(Ref<CameraBase>& camera)
		{
			renderPass->Render(camera);
		}

		virtual void DrawNode() override;
	
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
	};
}