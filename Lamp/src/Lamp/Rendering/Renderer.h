#pragma once
#include "OrthographicCameraController.h"
#include "Lamp/Rendering/SpriteBatch.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Rendering/Sprite.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Application.h"
#include <algorithm>

#include <LampEntity/Entity/Entity.h>

namespace Lamp
{
	class Renderer
	{
	public:
		static void Initialize();

		static void Begin(OrthographicCamera& camera);
		static void End();

		static void AddSprite(Sprite& sprite) { m_RenderSprites.push_back(sprite); sprite.SetPosition((uint32_t)m_RenderSprites.size() - 1); }
		static void RemoveSprite(Sprite& sprite) { m_RenderSprites.erase(m_RenderSprites.begin() + sprite.GetPosition()); }

		static void Draw(const std::shared_ptr<Shader>& shader, const LampEntity::IEntity* pEntity);
		static void SetClearColor(const glm::vec4& color) { glClearColor(color.r, color.g, color.g, color.a); glClearDepth(1.f); }
		static void Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

		static void BindFBO(uint32_t& fbo) { glBindFramebuffer(GL_FRAMEBUFFER, fbo); }
		static void UnbindFBO() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
		static void GenerateFrameBuffers(uint32_t& fbo) { glGenFramebuffers(1, &fbo); }
		static void CreateTexture(uint32_t& texture) 
		{
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

	private: 
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_pSceneData;
		static SpriteBatch* m_pSpriteBatch;
		static std::vector<Sprite> m_RenderSprites;
	};
}