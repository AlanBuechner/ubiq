#pragma once

#include "Engine/Core/Window.h"
#include "Engine/Renderer/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace Engine 
{

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override; // updates the window

		inline virtual unsigned int GetWidth() const override { return m_Data.Width; } // get the width of the window
		inline virtual unsigned int GetHeight() const override { return m_Data.Height; } // get the hight of the window

		// Window attributes
		inline virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled) override; // enables v-sync
		virtual bool IsVSync() const override; // returns wither or not v-sync is enabled

		virtual void SetViewport(float width, float height) override;

		inline virtual void* GetNativeWindow() const;

		virtual void SetTitle(const std::string& title) override;

		virtual void ToggleMinimize() override;
		virtual void ToggleMaximize() override;
		virtual void ToggleFullScreen() override;

	private:
		virtual void Init(const WindowProps& props); // initialize the window
		virtual void Shutdown() override; // closes the window

		static LRESULT WINAPI HandleEventSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // handles setting up events
		static LRESULT WINAPI HandleEventThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // sends events to the corresponding window to be handled
		LRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_Window;
		GraphicsContext* m_Context;

		class WindowClass
		{
		public:
			static const LPCWSTR GetName() noexcept { return wndClassName; }
			static HINSTANCE GetInstance() noexcept { return wndClass.hInst; }

		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			static constexpr const LPCWSTR wndClassName = TEXT("Window");
			static WindowClass wndClass;
			HINSTANCE hInst;
		};

		struct WindowData
		{
			std::string Title; // window title
			unsigned int Width, Height; // windows width and hight
			bool FullScreen, Maximized, Minimized;
			bool VSync; // v-sync

			EventCallbackFn EventCallback;
		};

		WINDOWPLACEMENT m_wpPrev = { sizeof(m_wpPrev) };
		WindowData m_Data;
	};

}
