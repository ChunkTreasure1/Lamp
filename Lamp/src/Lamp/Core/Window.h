#pragma once

#include <functional>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>
#include "Lamp/Event/Event.h"
#include "Lamp/Core/Timestep.h"

namespace Lamp
{
	struct WindowProps
	{
		WindowProps(std::string title = "Lamp", uint32_t width = 1280, uint32_t height = 720, bool isVSync = true)
			: Title(title), Width(width), Height(height), IsVSync(isVSync)
		{}

		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool IsVSync;
	};

	class Window
	{
	public:
		Window(WindowProps& props = WindowProps());
		~Window();

		using EventCallbackFn = std::function<void(Event&)>;

		//Functions
		void Init(const WindowProps& props);
		void SwapBuffer();
		void Update(Timestep ts);

		//Getting
		inline const uint32_t GetWidth() const { return m_Data.Width; }
		inline const uint32_t GetHeight() const { return m_Data.Height; }
		inline const bool GetIsVSync() const { return m_Data.VSync; }
		inline void* GetNativeWindow() const { return m_pWindow; }

		//Setting
		inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
		inline void SetIsVSync(bool state);
		void ShowCursor(bool state);

	private:
		//Member vars
		GLFWwindow* m_pWindow;

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


