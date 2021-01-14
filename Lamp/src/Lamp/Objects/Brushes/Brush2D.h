#pragma once

#include "Lamp/Core/Core.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/Renderer2D.h"

#include "Lamp/Objects/Object.h"
#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
	class Brush2D : public Object
	{
	public:
		Brush2D(const std::string& spritePath);

		virtual void OnEvent(Event& e) override;
		virtual void Destroy() override;
		virtual uint64_t GetEventMask() override { return EventType::AppRender; }

	public:
		static Brush2D* Create(const std::string& path);

	private:
		bool OnRender(AppRenderEvent& e);

	private:
		bool m_ShouldCollide;
		std::string m_SpritePath;
		Ref<Texture2D> m_Sprite;
	};
}