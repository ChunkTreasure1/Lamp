#pragma once

#include <Lamp/Rendering/Sprite.h>

#include "Component/EntityComponent.h"

namespace LampEntity
{
	class SpriteComponent : public IEntityComponent
	{
	public:
		//Base
		SpriteComponent();
		~SpriteComponent() override;

		virtual void Initialize() override;
		virtual void Update() override;

		virtual void OnEvent(Lamp::Event& event) override;
		virtual void Draw() override;

	private:
		GLuint m_Texture;
		glm::vec4 m_UVRect;

	};
}