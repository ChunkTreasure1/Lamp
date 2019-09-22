#pragma once
#include "Lamp/Rendering/Camera2D.h"
#include "Lamp/Rendering/SpriteBatch.h"
#include "Lamp/Rendering/Shader/Shader.h"

#include "Lamp/Timing.h"
#include "Lamp/Rendering/Sprite.h"
#include <algorithm>


namespace Lamp
{
	class Renderer
	{
	public:
		Renderer(Window* pWindow);
		~Renderer();

		void Draw();

		inline void AddSprite(Sprite& sprite) { m_RenderSprites.push_back(sprite); sprite.SetPosition(m_RenderSprites.size() - 1); }
		inline void RemoveSprite(Sprite& sprite) { m_RenderSprites.erase(m_RenderSprites.begin() + sprite.GetPosition()); }

	private:
		std::unique_ptr<Camera2D> m_pCamera;
		std::unique_ptr<SpriteBatch> m_pSpriteBatch;
		std::unique_ptr<Shader> m_pShader;
		std::unique_ptr<FPSLimiter> m_pFPSLimiter;

		Window* m_pWindow;
		std::vector<Sprite> m_RenderSprites;

		float m_Time;
		float m_FPS;
	};
}