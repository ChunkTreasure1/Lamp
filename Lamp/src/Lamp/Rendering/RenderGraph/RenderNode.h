#pragma once

#include <string>

#include "Lamp/Rendering/RenderPass.h"

namespace Lamp
{
	struct RenderNode;
	struct RenderLink;

	enum class RenderNodeType
	{
		Pass = 0,
		Framebuffer = 1,
		Texture = 2,
		DynamicUniform = 3
	};

	enum class RenderAttributeType
	{
		Texture,
		Framebuffer,
		DynamicUniform
	};

	struct RenderAttribute
	{
		virtual ~RenderAttribute() {}

		std::vector<Ref<RenderLink>> links;
		RenderNode* pNode;
		uint32_t id;
		std::string name;
		RenderAttributeType type;
	};

	struct RenderOutputAttribute : public RenderAttribute
	{};

	struct RenderInputAttribute : public RenderAttribute
	{
		std::any data;
		void* pData = nullptr;
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
		std::vector<Ref<RenderOutputAttribute>> outputs;
		std::vector<Ref<RenderInputAttribute>> inputs;

		uint32_t currId;
		uint32_t id;

		virtual void Initialize() = 0;
		virtual void Start() = 0;
		virtual void DrawNode() = 0;
		virtual void Activate(std::any value) = 0;
		virtual RenderNodeType GetNodeType() = 0;
	};

	struct RenderNodePass : public RenderNode
	{
		RenderNodePass()
		{
		}

		virtual ~RenderNodePass() override {}

		std::string name;
		Ref<RenderPass> renderPass;

		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override;
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Pass; }

	private:
		std::vector<const char*> m_Shaders;
	};

	struct RenderNodeFramebuffer : public RenderNode
	{
		RenderNodeFramebuffer()
		{

		}

		virtual ~RenderNodeFramebuffer() override {}

		Ref<Framebuffer> framebuffer;

		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Framebuffer; }
	};

	struct RenderNodeTexture : public RenderNode
	{
		RenderNodeTexture()
		{

		}

		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Texture; }

		Ref<Texture2D> texture;

	private:
		void GetTexture();
	};

	struct RenderNodeDynamicUniform : public RenderNode
	{
		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
		virtual RenderNodeType GetNodeType() { return RenderNodeType::DynamicUniform; }

		void* pData = nullptr;
		UniformType uniformType;
		std::string dataName;

	private:
		std::vector<const char*> m_Uniforms;
		int m_CurrentlySelectedUniform = 0;
	};
}