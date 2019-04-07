#pragma once
#include "UbiqWin.h"
#include "Engine/Window.h"

#ifdef PLATFORM_D3D11

namespace Engine
{
	class WindowsWindow : public Window
	{
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override; // updates the window

		inline virtual unsigned int GetWidth() const override { return m_Data.Width; } // get the width of the window
		inline virtual unsigned int GetHeight() const override { return m_Data.Height; } // get the hight of the window

		// Window attributes
		inline virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled) override; // enables v-sync
		virtual bool IsVSync() const override; // returns wither or not v-sync is enabled

		inline virtual void* GetNativeWindow() const;
	private:
		virtual void Init(const WindowProps& props); // initialize the window
		virtual void Shutdown(); // closes the window
	private:
		HWIND* m_Window;

		struct WindowData
		{
			std::string Title; // window title
			unsigned int Width, Height; // windows width and hight
			bool VSync; // v-sync

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}

#endif