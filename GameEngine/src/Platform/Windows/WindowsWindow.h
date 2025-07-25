#pragma once

#include "Engine/Core/Window.h"
#include "Engine/Renderer/Abstractions/Resources/SwapChain.h"
#include "Engine/Renderer/Abstractions/GraphicsContext.h"
#include <thread>

namespace Engine 
{

	class WindowsWindow : public Window
	{
	public:
		virtual ~WindowsWindow();

		virtual void Setup(const WindowProps& props) override;

		virtual void Close() override { m_RequestClose = true; }

		virtual void HandleEvents() override;

		virtual void OnUpdate() override; // updates the window

		inline virtual unsigned int GetWidth() const override { return m_Data.Width; } // get the width of the window
		inline virtual unsigned int GetHeight() const override { return m_Data.Height; } // get the hight of the window

		// Window attributes
		virtual void SetVSync(bool enabled) override; // enables v-sync
		virtual bool IsVSync() const override; // returns wither or not v-sync is enabled

		virtual bool GetFullScreen() const override;
		virtual bool GetMaximised() const override;
		virtual bool GetMinimized() const override;

		virtual void SetViewport(float width, float height) override;

		virtual void* GetNativeWindow() const override;

		virtual void SetTitle(const std::string& title) override;
		virtual const std::string& GetTitle() const override;

		virtual void ToggleMinimize() override;
		virtual void ToggleMaximize() override;
		virtual void ToggleFullScreen() override;
		
		virtual Ref<SwapChain> GetSwapChain() const override { return m_SwapChain; }

	private:
		static LRESULT WINAPI HandleEventSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // handles setting up events
		static LRESULT WINAPI HandleEventThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); // sends events to the corresponding window to be handled
		LRESULT HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND m_Window;
		Ref<SwapChain> m_SwapChain;

		struct WindowData
		{
			std::string Title; // window title
			uint32 Width, Height; // windows width and hight
			bool FullScreen, Maximized, Minimized;
			bool VSync; // v-sync
		};

		WINDOWPLACEMENT m_wpPrev = { sizeof(m_wpPrev) };
		WindowData m_Data;

		bool m_RequestClose = false;

		struct {
			RECT close;
			RECT maximize;
			RECT minimize;
		} m_CustomTitleBarButtonRects;

		friend Window;
	};

}
