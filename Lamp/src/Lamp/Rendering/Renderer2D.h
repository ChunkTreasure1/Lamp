#pragma once

#include <glm/glm.hpp>
#include "Texture2D/Texture2D.h"
#include "Cameras/OrthographicCamera.h"

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/Vertices/FrameBuffer.h"

namespace Lamp
{
	class Renderer2D
	{
	public:

		static void Initialize();
		static void Shutdown();

		static void Begin(const Ref<CameraBase>& camera);
		static void End();
		static void Flush();

		static void DrawQuad(const glm::mat4& tm, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& tm, const Ref<Texture2D>& texture, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f });

		static void DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const glm::vec4& color);

		static void DrawLine(const glm::vec3& posA, const glm::vec3& posB);
		static void DrawLine(const glm::vec2& posA, const glm::vec2& posB);

		static Ref<FrameBuffer>& GetFrameBuffer() { return m_pFrameBuffer; }

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t LineCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};
		static Statistics GetStats();
		static void ResetStats();

	private:
		static void StartNewBatch();
		static void ResetBatchData();

	private:
		static Ref<FrameBuffer> m_pFrameBuffer;
	};
}