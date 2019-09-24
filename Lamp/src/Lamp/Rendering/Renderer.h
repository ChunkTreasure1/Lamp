#pragma once
#include "OrthographicCameraController.h"
#include "Lamp/Rendering/SpriteBatch.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Rendering/Sprite.h"
#include "Lamp/Event/Event.h"
#include <algorithm>


namespace Lamp
{
	class Renderer
	{
	public:
		static void Initialize();

		static void Begin(OrthographicCamera& camera);
		static void End();

		static void AddSprite(Sprite& sprite) { m_RenderSprites.push_back(sprite); sprite.SetPosition(m_RenderSprites.size() - 1); }
		static void RemoveSprite(Sprite& sprite) { m_RenderSprites.erase(m_RenderSprites.begin() + sprite.GetPosition()); }

		static void Draw(const std::shared_ptr<Shader>& shader, const Sprite& sprite);
		static void SetClearColor(const glm::vec4& color) { glClearColor(color.r, color.g, color.g, color.a); glClearDepth(1.f); }
		static void Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

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