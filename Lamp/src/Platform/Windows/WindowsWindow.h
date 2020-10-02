#pragma once

#include "Lamp/Core/Window.h"
#include "Lamp/Rendering/GraphicsContext.h"

namespace Lamp
{
	class WindowsWindow : public Window 
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow() override;

		virtual void Update(Timestep ts) override;
		
		//Getting
		virtual const uint32_t GetWidth() const override { return m_Data.Width; }
		virtual const uint32_t GetHeight() const override { return m_Data.Height; }
		virtual const bool GetIsVSync() const override { return m_Data.VSync; }
		virtual void* GetNativeWindow() const override { return m_pWindow; }
		inline Scope<GraphicsContext>& GetGraphicsContext() { return m_pContext; }

		//Setting
		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetIsVSync(bool state) override;
		virtual void ShowCursor(bool state) override;
		virtual void SetSize(const glm::vec2& size) override;

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_pWindow;
		Scope<GraphicsContext> m_pContext;

		struct WindowData
		{
			std::string Title;
			uint32_t Width;
			uint32_t Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}