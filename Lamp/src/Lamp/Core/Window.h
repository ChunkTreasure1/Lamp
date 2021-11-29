#pragma once

#include <functional>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>
#include "Lamp/Event/Event.h"
#include "Lamp/Core/Time/Timestep.h"

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
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;
		virtual void Update(Timestep ts) = 0;
		virtual void Maximize() = 0;

		//Getting
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const bool GetIsVSync() const = 0;
		virtual void* GetNativeWindow() const = 0;
		virtual const Ref<GraphicsContext> GetGraphicsContext() const = 0;

		//Setting
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetIsVSync(bool state) = 0;
		virtual void ShowCursor(bool state) = 0;
		virtual void SetSize(const glm::vec2& size) = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}


