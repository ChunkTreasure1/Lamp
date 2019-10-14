#pragma once

#include "Lamp/Rendering/Sprite.h"
#include "Lamp/Rendering/Texture/GLTexture.h"
#include "Lamp/Entity/Base/Component.h"
#include "Lamp/Input/ResourceManager.h"

namespace Lamp
{
	class SpriteComponent final : public IEntityComponent
	{
	public:
		//Base
		SpriteComponent(const std::string& path)
			: m_Depth(1.f), m_UVRect(0, 0, 1, 1), m_Path(path)
		{
			m_SpriteValues.Depth = m_Depth;
			m_SpriteValues.Path = m_Path;
		}
		~SpriteComponent() {}

		virtual void Initialize() override 
		{
			if (m_Path.length() > 0)
			{
				m_Texture = ResourceManager::GetTexture(m_Path);
			}
			else
			{
				return;
			}
		}
		virtual void Update() override {}
		virtual void Draw() override {}

		//Setting
		inline void SetPath(const std::string& path) 
		{ 
			m_Path = path; 
			m_Texture = ResourceManager::GetTexture(m_Path);
		}
		inline void SetDepth(float val) { m_Depth = val; }
		inline void SetColor(Color col) { m_Color = col; }

		inline void SetUVRect(glm::vec4 uvRect) { m_UVRect = uvRect; }

		//Getting
		inline const GLTexture& GetTexture() const { return m_Texture; }
		inline const float GetDepth() const { return m_Depth; }
		inline const Color GetColor() const { return m_Color; }

		inline const glm::vec4 GetUVRect() const { return m_UVRect; }
		virtual const EditorValues GetEditorValues() const { return m_SpriteValues; }

	private:
		std::string m_Path;
		glm::vec4 m_UVRect;

		float m_Depth;
		GLTexture m_Texture;
		Color m_Color;

		struct SpriteValues : EditorValues
		{
			SpriteValues()
				: EditorValues("Sprite component"), Depth(1.f)
			{}

			std::string Path;
			float Depth;
		};

		SpriteValues m_SpriteValues = SpriteValues();
	};
}