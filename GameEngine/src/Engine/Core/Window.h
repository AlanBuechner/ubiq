#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Events/Event.h"

namespace Engine {

	struct WindowProps
	{
		std::string Title;
		uint32 Width;
		uint32 Height;
		bool Maximized;
		bool VSync;

		WindowProps(const std::string& title = "Game Engine", uint32 width = 1280, uint32 height = 720, bool maximized = false, bool vSync = true) :
			Title(title), Width(width), Height(height), Maximized(maximized), VSync(vSync)
		{}
	};

	// Interface representing a desktop system based Window
	class ENGINE_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}
		virtual void Shutdown() = 0;

		virtual void OnUpdate() = 0; // updates the window

		virtual uint32 GetWidth() const = 0; // gets the width of the window
		virtual uint32 GetHeight() const = 0; // gets the hight of the window

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void SetViewport(float width, float height) = 0;

		virtual void SetTitle(const std::string& title) = 0;

		virtual void ToggleMinimize() = 0;
		virtual void ToggleMaximize() = 0;
		virtual void ToggleFullScreen() = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProps& props = WindowProps()); // creates a new window
	};

}
