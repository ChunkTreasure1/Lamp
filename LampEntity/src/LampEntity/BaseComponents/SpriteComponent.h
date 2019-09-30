#pragma once

#include <Lamp/Rendering/Sprite.h>
#include <Lamp/Rendering/Texture/GLTexture.h>

#include "LampEntity/Component/EntityComponent.h"

namespace LampEntity
{
	class SpriteComponent final : public IEntityComponent
	{
	public:
		//Base
		SpriteComponent()
			: m_Depth(1.f), m_UVRect(0, 0, 1, 1)
		{}

		virtual ~SpriteComponent() override {};

		virtual void Initialize() override;
		virtual void Update() override {}

		virtual void OnEvent(Lamp::Event& event) override {}
		virtual void Draw() override {}

		//Setting
		inline void SetPath(const std::string& path) { m_Path = path; }
		inline void SetDepth(float val) { m_Depth = val; }
		inline void SetColor(Lamp::Color col) { m_Color = col; }

		inline void SetUVRect(glm::vec4 uvRect) { m_UVRect = uvRect; }

		//Getting
		inline const Lamp::GLTexture& GetTexture() const { return m_Texture; }
		inline const float GetDepth() const { return m_Depth; }
		inline const Lamp::Color GetColor() const { return m_Color; }
		
		inline const glm::vec4 GetUVRect() const { return m_UVRect; }

	private:
		std::string m_Path;
		glm::vec4 m_UVRect;

		float m_Depth;
		Lamp::GLTexture m_Texture;
		Lamp::Color m_Color;
	};
}