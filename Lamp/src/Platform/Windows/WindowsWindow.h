#pragma once

#include "Lamp/Core/Window.h"
#include "Lamp/Rendering/GraphicsContext.h"

namespace Lamp
{
	class WindowsWindow : public Window 
	{
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow() override;

		void Update(Timestep ts) override;
		void Maximize() override;

		//Getting
		const uint32_t GetWidth() const override { return m_Data.Width; }
		const uint32_t GetHeight() const override { return m_Data.Height; }
		const bool GetIsVSync() const override { return m_Data.VSync; }
		void* GetNativeWindow() const override { return m_pWindow; }
		const Ref<GraphicsContext> GetGraphicsContext() const override { return m_pContext; }

		//Setting
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetIsVSync(bool state) override;
		void ShowCursor(bool state) override;
		void SetSize(const glm::vec2& size) override;

	private:
		void Init(const WindowProps& props);
		void Shutdown();

	private:
		GLFWwindow* m_pWindow;
		Ref<GraphicsContext> m_pContext;

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