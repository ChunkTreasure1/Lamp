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
		Renderer();
		~Renderer();

		void Draw(Timestep ts);
		void OnEvent(Event& e);

		inline void AddSprite(Sprite& sprite) { m_RenderSprites.push_back(sprite); sprite.SetPosition(m_RenderSprites.size() - 1); }
		inline void RemoveSprite(Sprite& sprite) { m_RenderSprites.erase(m_RenderSprites.begin() + sprite.GetPosition()); }

	private:
		OrthographicCameraController m_CameraController;
		std::unique_ptr<SpriteBatch> m_pSpriteBatch;
		std::unique_ptr<Shader> m_pShader;

		std::vector<Sprite> m_RenderSprites;
	};
}