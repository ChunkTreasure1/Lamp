#pragma once

#include <glm/glm.hpp>
#include "Cameras/OrthographicCamera.h"

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/Vertices/Framebuffer.h"

namespace Lamp
{
	class Renderer2D
	{
	public:

		static void Initialize();
		static void Shutdown();

		static void BeginPass();
		static void EndPass();

		static void Flush();

		static void DrawQuad(const glm::mat4& tm, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& tm, const Ref<Material> mat, uint32_t id, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f });

		static void SubmitQuad(const glm::mat4& transform, Ref<Material> mat, size_t id = -1);
		static void DrawRenderBuffer();

		struct Statistics
		{
			uint32_t drawCalls = 0;
			uint32_t quadCount = 0;

			uint32_t GetTotalVertexCount() { return quadCount * 4; }
			uint32_t GetTotalIndexCount() { return quadCount * 6; }
		};

		static Statistics GetStats();
		static void ResetStats();

	private:
		static void StartNewBatch();
		static void ResetBatchData();

		static RenderBuffer s_RenderBuffer;
	};
}