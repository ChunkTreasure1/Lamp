#pragma once

#include <glm/glm.hpp>
#include "Texture2D/Texture2D.h"
#include "OrthographicCamera.h"

namespace Lamp
{
	class Renderer2D
	{
	public:

		static void Initialize();
		static void Shutdown();

		static void Begin(const OrthographicCamera& camera);
		static void End();

		static void DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const glm::vec4& color);

		static void DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const Ref<Texture2D>& texture);
		static void DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const Ref<Texture2D>& texture);
	};
}