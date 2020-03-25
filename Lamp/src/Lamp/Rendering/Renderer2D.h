#pragma once

#include <glm/glm.hpp>
#include "Texture2D/Texture2D.h"
#include "Cameras/OrthographicCamera.h"

#include "Lamp/Core/Core.h"

namespace Lamp
{
	class Renderer2D
	{
	public:

		static void Initialize();
		static void Shutdown();

		static void Begin(const OrthographicCamera camera);
		static void End();

		static void DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const glm::vec4& color);

		static void DrawQuad(const glm::vec3& pos, const glm::vec2& scale, float rotation, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& pos, const glm::vec2& scale, float rotation, const glm::vec4& color);

		static void DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const Ref<Texture2D>& texture, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const Ref<Texture2D>& texture, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f });

		static void DrawQuad(const glm::vec3& pos, const glm::vec2& scale, float rotation, const Ref<Texture2D>& texture, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawQuad(const glm::vec2& pos, const glm::vec2& scale, float rotation, const Ref<Texture2D>& texture, const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f });
	
		static void DrawLine(const glm::vec3& posA, const glm::vec3& posB);
		static void DrawLine(const glm::vec2& posA, const glm::vec2& posB);
	};
}