#pragma once

#include "Vertices/Framebuffer.h"
#include "Lamp/Core/Core.h"
#include "RenderCommand.h"
#include "Renderer3D.h"

#include "RenderGraph/RenderPassSpecifications.h"

#define LP_EXTRA_RENDER(fn) std::bind(&fn, this)

namespace Lamp
{
	enum class ClearType : uint32_t
	{
		None = 0,
		Color = 1,
		Depth = 2,
		ColorDepth = 3
	};

	enum class DrawType : uint32_t
	{
		All = 0,
		Quad = 1,
		Line = 2,
		Forward = 3,
	};

	struct RenderPassSpecification
	{
		using RenderFunc = std::function<void()>;
		std::vector<RenderFunc> ExtraRenders;

		Ref<Framebuffer> TargetFramebuffer;

		uint32_t LightIndex = 0;
		std::string Name = "";

		ClearType clearType = ClearType::ColorDepth;
		CullFace cullFace = CullFace::Back;
	 	DrawType drawType = DrawType::All;
		Ref<Shader> renderShader = nullptr; // if null it will use the material shader

		std::map<GraphUUID, PassStaticUniformSpecification> staticUniforms; // name, type, data
		std::map<GraphUUID, std::pair<PassDynamicUniformSpecification, GraphUUID>> dynamicUniforms; // name, type, data, attrId
		std::map<GraphUUID, std::pair<PassTextureSpecification, GraphUUID>> textures; // texture, texBindSlot, attrId
		std::map<GraphUUID, std::pair<PassFramebufferSpecification, GraphUUID>> framebuffers; // framebuffer, GraphFramebufferSpec, attrId
		std::map<GraphUUID, std::pair<PassFramebufferCommandSpecification, GraphUUID>> framebufferCommands; // main buffer, secondary buffer, command, attrId
	};

	class RenderPass
	{
	public:
		friend class RenderPassManager;

		RenderPass()
			: m_ID(0)
		{}
		RenderPass(const RenderPassSpecification& spec);
		~RenderPass() {}

		inline uint32_t GetID() { return m_ID; }
		inline RenderPassSpecification& GetSpecification() { return m_PassSpec; }

		void Render(Ref<CameraBase>& camera);

	private:
		inline void SetID(uint32_t id) { m_ID = id; }

	private:
		uint32_t m_ID;
		RenderPassSpecification m_PassSpec;
	};

	class RenderPassManager
	{
	public:
		RenderPassManager() = default;
		~RenderPassManager()
		{
			m_RenderPasses.clear();
			s_Instance = nullptr;
		}

		void AddPass(Ref<RenderPass>& pass);
		bool RemovePass(uint32_t id);

		std::vector<Ref<RenderPass>>& GetRenderPasses();

		void RenderPasses(Ref<CameraBase>& camera);

	public:
		static Ref<RenderPassManager>& Get()
		{
			if (s_Instance.get() == nullptr)
			{
				s_Instance = CreateRef<RenderPassManager>();
			}
			return s_Instance;
		}

	private:
		std::vector<Ref<RenderPass>> m_RenderPasses;

	private:
		static Ref<RenderPassManager> s_Instance;
	};
}