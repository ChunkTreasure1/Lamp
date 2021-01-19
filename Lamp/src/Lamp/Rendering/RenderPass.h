#pragma once

#include "Vertices/FrameBuffer.h"
#include "Lamp/Core/Core.h"
#include "RenderCommand.h"
#include "Renderer3D.h"

#define LP_EXTRA_RENDER(fn) std::bind(&fn, this)

namespace Lamp
{
	struct RenderPassInfo
	{
		Ref<CameraBase> Camera;
		glm::vec4 ClearColor;

		glm::mat4 ViewProjection = glm::mat4(1.f);
		glm::mat4 LightViewProjection = glm::mat4(1.f);

		bool IsShadowPass;
	};

	class RenderPass
	{
	public:
		using RenderFunc = std::function<void()>;

		friend class RenderPassManager;

		RenderPass(Ref<FrameBuffer>& frameBuffer, const RenderPassInfo& passInfo, std::vector<RenderFunc> extraRenders = {});
		~RenderPass()
		{
			m_ExtraRenders.clear();
		}

		inline uint32_t GetID() { return m_ID; }

		void Render();

	private:
		inline void SetID(uint32_t id) { m_ID = id; }

	private:
		Ref<FrameBuffer> m_FrameBuffer;
		std::vector<RenderFunc> m_ExtraRenders;

		uint32_t m_ID;
		RenderPassInfo m_PassInfo;
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

		void RenderPasses();

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