#include "pch.h"
#include "WindowsWindow.h"
#include "Engine/Core/Application.h"
#include "Engine/Util/Performance.h"

#include "Engine/Core/Input/InputControler.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/Input/Input.h"

#include "Engine/Util/Utils.h"

#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#pragma comment(lib, "uxtheme.lib")

#include <imgui.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Engine
{
	void Window::Init()
	{
		WNDCLASSEX mainClass = { 0 };
		mainClass.cbSize = sizeof(mainClass);
		mainClass.style = CS_CLASSDC;
		mainClass.lpfnWndProc = WindowsWindow::HandleEventSetup;
		mainClass.cbClsExtra = 0;
		mainClass.cbWndExtra = 0;
		mainClass.hInstance = GetModuleHandle(nullptr);
		mainClass.hIcon = nullptr;
		mainClass.hIconSm = nullptr;
		mainClass.hbrBackground = nullptr;
		mainClass.hCursor = nullptr;
		mainClass.lpszMenuName = nullptr;
		mainClass.lpszClassName = TEXT("Main Window");
		RegisterClassEx(&mainClass);

		WNDCLASSEX childClass;
		childClass.cbSize = sizeof(WNDCLASSEX);
		childClass.style = CS_HREDRAW | CS_VREDRAW;
		childClass.lpfnWndProc = WindowsWindow::HandleEventSetup;
		childClass.cbClsExtra = 0;
		childClass.cbWndExtra = 0;
		childClass.hInstance = ::GetModuleHandle(NULL);
		childClass.hIcon = NULL;
		childClass.hCursor = NULL;
		childClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
		childClass.lpszMenuName = NULL;
		childClass.lpszClassName = TEXT("Child Window");
		childClass.hIconSm = NULL;
		RegisterClassEx(&childClass);

	}

	void Window::Shutdown()
	{
		UnregisterClass(TEXT("Main Window"), GetModuleHandle(nullptr));
		UnregisterClass(TEXT("Child Window"), GetModuleHandle(nullptr));
	}

	void Window::HandleEvents()
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		CREATE_PROFILE_FUNCTIONI();
		auto time = CREATE_PROFILEI();
		m_Data.Title = props.Title; // sets the window title
		m_Data.Width = props.Width; // sets the width of the window
		m_Data.Height = props.Height; // sets the hight of the window
		m_Data.Maximized = props.Maximized;
		m_Data.VSync = props.VSync;

		m_Data.FullScreen = false;
		m_Data.Minimized = false;

		CORE_INFO("Creating window {0} ({1}, {2}), {3}", props.Title, props.Width, props.Height, props.Maximized);

		RECT wr;
		if (props.Maximized)
			SystemParametersInfo(SPI_GETWORKAREA, 0, &wr, 0);
		else
		{
			wr.left = 100;
			wr.right = props.Width + wr.left;
			wr.top = 100;
			wr.bottom = props.Height + wr.top;
		}

		// adjust the window rect for the styles that it has
		DWORD style = props.Parent ?
			WS_POPUP :
			WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
		DWORD styleex = props.Parent ? WS_EX_APPWINDOW : 0;
		CORE_ASSERT(AdjustWindowRect(&wr, style, FALSE) != 0, "Failed to adjust window rect");

		// creat the window
		HWND parent = props.Parent ? (HWND)props.Parent->GetNativeWindow() : nullptr;
		const wchar_t* wtitle = GetWStr(props.Title.c_str());
		m_Window = CreateWindowEx(
			styleex, props.Parent ? TEXT("Child Window") : TEXT("Main Window"), wtitle, style,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
			parent, nullptr, GetModuleHandle(nullptr), this
		);
		delete[] wtitle;

		// check if the windows was created successfully
		CORE_ASSERT(m_Window != nullptr, "Failed to create window");

		// show the window
		ShowWindow(m_Window, props.Maximized ? SW_MAXIMIZE : SW_SHOW);
		UpdateWindow(m_Window);

		// Register raw input

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = m_Window;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));

		// set up swap chain
		m_SwapChain = SwapChain::Create(*this);
		m_SwapChain->Init(3);
		m_SwapChain->SetVSync(m_Data.VSync);
	}

	WindowsWindow::~WindowsWindow()
	{
		Destroy(); // closes the window
	}

	void WindowsWindow::Destroy()
	{
		if (m_Window)
		{
			m_SwapChain->CleanUp();
			m_SwapChain.reset();
			DestroyWindow(m_Window);
			m_Window = nullptr;
		}
	}

	void WindowsWindow::OnUpdate()
	{
		m_SwapChain->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		m_Data.VSync = enabled;
		m_SwapChain->SetVSync(enabled);
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync; // returns the v-sync state
	}

	bool WindowsWindow::GetFullScreen() const
	{
		return m_Data.FullScreen;
	}

	bool WindowsWindow::GetMaximised() const
	{
		return m_Data.Maximized;
	}

	bool WindowsWindow::GetMinimized() const
	{
		return m_Data.Minimized;
	}

	void WindowsWindow::SetViewport(float width, float height)
	{
		if (m_Data.FullScreen || m_Data.Maximized || m_Data.Minimized) return;
		SetWindowPos(m_Window, nullptr, 0, 0, (int)width, (int)height, 0);
	}

	inline void* WindowsWindow::GetNativeWindow() const
	{
		return m_Window;
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{
		SetWindowTextA(m_Window, title.c_str());
	}

	const std::string& WindowsWindow::GetTitle() const
	{
		return m_Data.Title;
	}

	void WindowsWindow::ToggleMinimize()
	{
		ShowWindow(m_Window, m_Data.Minimized ? (m_Data.Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL) : SW_SHOWMINIMIZED);
		m_Data.Minimized = !m_Data.Minimized;
	}

	void WindowsWindow::ToggleMaximize()
	{
		ShowWindow(m_Window, m_Data.Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
		m_Data.Maximized = !m_Data.Maximized;
	}

	void WindowsWindow::ToggleFullScreen()
	{
		DWORD dwStyle = GetWindowLong(m_Window, GWL_STYLE);
		if (!m_Data.FullScreen)
		{
			MONITORINFO mi = { sizeof(mi) };
			if (GetWindowPlacement(m_Window, &m_wpPrev) && GetMonitorInfo(MonitorFromWindow(m_Window, MONITOR_DEFAULTTOPRIMARY), &mi))
			{
				SetWindowLong(m_Window, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(m_Window, HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED
				);

				m_Data.FullScreen = true;
				m_Data.Minimized = false;
			}
		}
		else
		{
			SetWindowLong(m_Window, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
			SetWindowPlacement(m_Window, &m_wpPrev);

			ShowWindow(m_Window, m_Data.Minimized ? SW_SHOWMINIMIZED : (m_Data.Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL));
			m_Data.FullScreen = false;
		}
	}

	LRESULT WINAPI WindowsWindow::HandleEventSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// send events to imgui
		if (Application::InEditer())
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

		if (msg == WM_NCCREATE)
		{
			// get the data the was sent from CreateWindow
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			WindowsWindow* const pWnd = static_cast<WindowsWindow*>(pCreate->lpCreateParams); // get the window

			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd)); // set user data on the window to contain the Window class
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowsWindow::HandleEventThunk)); // change the event handler to handle event thunk

			return pWnd->HandleEvent(hWnd, msg, wParam, lParam); // handle the event
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI WindowsWindow::HandleEventThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// send events to imgui
		if (Application::InEditer())
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

		// get a pointer to the window
		WindowsWindow* const pWnd = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return pWnd->HandleEvent(hWnd, msg, wParam, lParam); // call handle event for that window
	}

	LRESULT WindowsWindow::HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
			// keyboard events
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: // key down
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new KeyPressedEvent(static_cast<unsigned char>(wParam)));
			break;
		case WM_SYSKEYUP:
		case WM_KEYUP: // key up
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new KeyReleasedEvent(static_cast<unsigned char>(wParam)));
			break;
		case WM_CHAR:
			// TODO
			break;

			// mouse
		case WM_MOUSEMOVE: // mouse move
		{
			if (m_Data.EventCallback == nullptr) break;
			POINTS pt = MAKEPOINTS(lParam);
			m_Data.EventCallback(*new MouseMovedEvent(MOUSE_POSITON, pt.x, pt.y));
			//Math::Vector2 PrevMousePos = Input::GetPreviousMousePosition();
			//MouseMovedEvent* deltaMousePostionEvent = new MouseMovedEvent(MOUSE_DELTA, (float)pt.x - PrevMousePos.x, (float)pt.y - PrevMousePos.y); // creates new mouse moved event
			//m_Data.EventCallback(*deltaMousePostionEvent);
			break;
		}

		case WM_LBUTTONDOWN: // left mouse down
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new MouseButtonPressedEvent(VK_LBUTTON));
			break;
		case WM_RBUTTONDOWN: // right mouse down
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new MouseButtonPressedEvent(VK_RBUTTON));
			break;
		case WM_MBUTTONDOWN: // middle mouse down
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new MouseButtonPressedEvent(VK_MBUTTON));
			break;

		case WM_LBUTTONUP: // left moues up
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new MouseButtonReleasedEvent(VK_LBUTTON));
			break;
		case WM_RBUTTONUP: // right mouse up
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new MouseButtonReleasedEvent(VK_RBUTTON));
			break;
		case WM_MBUTTONUP: // middle mouse up
			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new MouseButtonReleasedEvent(VK_MBUTTON));
			break;
		case WM_INPUT: {
			unsigned size = 0;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
			RAWINPUT* raw = new RAWINPUT[size];
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));

			if (raw[0].header.dwType == RIM_TYPEMOUSE && (raw[0].data.mouse.lLastX != 0 || raw[0].data.mouse.lLastY != 0)) {
				MouseMovedEvent* deltaMousePostionEvent = new MouseMovedEvent(MOUSE_DELTA, (float)raw[0].data.mouse.lLastX, (float)raw[0].data.mouse.lLastY); // creates new mouse moved event
				m_Data.EventCallback(*deltaMousePostionEvent);
			}
			delete[] raw;
			CORE_INFO("hello");
			break;
		}


			// window events
		case WM_SIZE: // windows resize event
		{
			// set the window min and max bools and let DefWindowProc change the window state
			switch (wParam)
			{
			case SIZE_MAXIMIZED:
				m_Data.Maximized = true;
				m_Data.Minimized = false;
				break;
			case SIZE_MINIMIZED:
				m_Data.Minimized = true;
				break;
			case SIZE_RESTORED:
				m_Data.Minimized = false;
				break;

			default:
				break;
			}

			// set the width and hight
			m_Data.Width = LOWORD(lParam);
			m_Data.Height = HIWORD(lParam);
			// resize swap chain

			if (m_Data.EventCallback == nullptr) break;
			m_Data.EventCallback(*new WindowResizeEvent(m_Data.Width, m_Data.Height));
			m_SwapChain->Resize(m_Data.Width, m_Data.Height);
			break;
		}
		case WM_KILLFOCUS: // window remove focuses event
			break;
		case WM_CLOSE: // window close event
			if (m_Data.EventCallback != nullptr)
				m_Data.EventCallback(*new WindowCloseEvent());
			return 0; // stop DefWindowProc from being called
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

}
