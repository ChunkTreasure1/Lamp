#pragma once

#include "Vertices/Framebuffer.h"
#include "Lamp/Core/Core.h"
#include "RenderCommand.h"
#include "Renderer3D.h"

#include <any>

#define LP_EXTRA_RENDER(fn) std::bind(&fn, this)

namespace Lamp
{
	enum class PassType
	{
		DirShadow = 0,
		PointShadow = BIT(1),
		Lightning = BIT(2),
		Geometry = BIT(3),
		SSAO = BIT(4),
		SSAOBlur = BIT(5),
		Forward = BIT(6),
		Selection = BIT(7),
	};

	enum class ClearType
	{
		None = 0,
		Color = 1,
		Depth = 2,
		ColorDepth = 3
	};

	enum class DrawType
	{
		All = 0,
		Quad = 1,
		Line = 2
	};

	enum class TextureType
	{
		Color = 0,
		Depth = 1
	};

	enum class FramebufferCommand
	{
		Copy = 0
	};

	struct RenderPassSpecification
	{
		using RenderFunc = std::function<void()>;
		std::vector<RenderFunc> ExtraRenders;

		Ref<Framebuffer> TargetFramebuffer;

		PassType type;
		uint32_t LightIndex = 0;
		std::string Name = "";

		ClearType clearType = ClearType::ColorDepth;
		CullFace cullFace = CullFace::Back;
		DrawType drawType = DrawType::All;
		Ref<Shader> renderShader = nullptr; // if null it will use the material shader

		std::vector<std::tuple<std::string, UniformType, std::any>> staticUniforms; // name, type, data
		std::vector<std::tuple<std::string, UniformType, void*>> dynamicUniforms; // name, type, data
		std::vector<std::pair<Ref<Texture2D>, uint32_t>> textures; // texture, texBindSlot
		std::vector<std::tuple<Ref<Framebuffer>, TextureType, uint32_t, uint32_t>> framebuffers; // framebuffer, texture type, texBindSlot, attachId
		std::vector<std::tuple<Ref<Framebuffer>, Ref<Framebuffer>, FramebufferCommand>> framebufferCommands; // main buffer, secondary buffer, command
	};

	class RenderPass
	{
	public:
		friend class RenderPassManager;

		RenderPass() = default;
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