#pragma once

#include <Lamp/Rendering/Texture2D/Texture2D.h>
#include <Lamp/Entity/Base/BaseComponent.h>

#include <Lamp/Rendering/Renderer2D.h>

#include <Lamp/Core/Core.h>
#include <Lamp/Entity/Base/Entity.h>

namespace Lamp
{
	class SpriteComponent final : public Lamp::IEntityComponent
	{
	public:
		//Base
		SpriteComponent()
			: IEntityComponent("Sprite Component"), m_TextureTint(glm::vec4(1.f))
		{
			SetComponentProperties
			({
				{ PropertyType::String, "Path", static_cast<void*>(&m_Path) },
				{ PropertyType::Color, "Texture Tint", static_cast<void*>(&m_TextureTint) }
				});
		}
		~SpriteComponent() {}

		virtual void Initialize() override
		{
			if (m_Path.length() > 0)
			{
				m_Texture = Texture2D::Create(m_Path);
			}
			else
			{
				return;
			}
		}
		virtual void Update(Timestep ts) override {}
		virtual void Draw() override
		{
			Renderer2D::DrawQuad(m_pEntity->GetPosition(), glm::vec2(m_pEntity->GetScale().x, m_pEntity->GetScale().y), m_pEntity->GetRotation().x, m_Texture, m_TextureTint);
		}
		virtual void SetProperty(ComponentProperty& prop, void* pData) override
		{
			if (prop.Name == "Path")
			{
				char* p = static_cast<char*>(pData);
				SetTexture(p);

				delete p;
			}
			else if (prop.Name == "Texture Tint")
			{
				glm::vec4* p = std::any_cast<glm::vec4*>(ComponentProperties::GetValue(prop, pData));
				SetTextureTint({ p->x, p->y, p->z, p->w });

				delete p;
			}
		}

		//Setting
		inline void SetTexture(const std::string& path)
		{
			m_Path = path;
			m_Texture = Texture2D::Create(path);
		}
		inline void SetTexture(const char* path)
		{
			//Need to be improved to work in different way

			m_Path = std::string(path);
			m_Texture = Texture2D::Create(path);
		}
		inline void SetTextureTint(const glm::vec4& color) { m_TextureTint = color; }

		//Getting
		inline const Ref<Texture2D> GetTexture() const { return m_Texture; }

	public:
		static Ref<IEntityComponent> Create() { return std::make_shared<SpriteComponent>(); }
		static std::string GetFactoryName() { return "SpriteComponent"; }

	private:
		static bool s_Registered;

	private:
		std::string m_Path;
		std::shared_ptr<Texture2D> m_Texture;
		glm::vec4 m_TextureTint;
	};
}