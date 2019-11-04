#pragma once

#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Entity/Base/Component.h"
#include "TransformComponent.h"

#include "Lamp/Rendering/Renderer2D.h"

#include "Lamp/Core/Core.h"

namespace Lamp
{
	class SpriteComponent final : public IEntityComponent
	{
	public:
		//Base
		SpriteComponent(const std::string& path)
			: m_Path(path)
		{
			m_SpriteValues.Path = m_Path;
		}
		~SpriteComponent() {}

		virtual void Initialize() override 
		{
			if (m_Path.length() > 0)
			{
				m_Texture.reset(Texture2D::Create(m_Path));
			}
			else
			{
				return;
			}
		}
		virtual void Update() override {}
		virtual void Draw() override 
		{
			std::vector<IEntityComponent*> pComps = GetOwner()->GetComponents();
			for (auto& pC : pComps)
			{
				TransformComponent* pTrans = dynamic_cast<TransformComponent*>(pC);
				if (pTrans)
				{
					Renderer2D::DrawQuad(pTrans->GetPosition(), glm::vec2(pTrans->GetScale().x, pTrans->GetScale().y), m_Texture);
				}
			}
		}

		//Setting
		inline void SetTexture(const std::string& path) 
		{ 
			m_Path = path; 
			m_Texture.reset(Texture2D::Create(path));
		}

		//Getting
		inline const Ref<Texture2D> GetTexture() const { return m_Texture; }
		virtual const EditorValues GetEditorValues() const { return m_SpriteValues; }

	private:
		std::string m_Path;
		std::shared_ptr<Texture2D> m_Texture;

		struct SpriteValues : EditorValues
		{
			SpriteValues()
				: EditorValues("Sprite component")
			{}

			std::string Path;
		};

		SpriteValues m_SpriteValues = SpriteValues();
	};
}