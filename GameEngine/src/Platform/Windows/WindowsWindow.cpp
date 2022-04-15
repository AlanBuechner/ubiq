#include "pch.h"
#include "WindowsWindow.h"
#include "Engine/Core/Log.h"
#include "Engine/Util/Performance.h"

#include "Engine/Core/Input/InputControler.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/Input/Input.h"

#include "Engine/Util/Utils.h"

#include <imgui.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Engine::WindowsWindow::WindowClass Engine::WindowsWindow::WindowClass::wndClass;

namespace Engine 
{
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		CREATE_PROFILE_FUNCTIONI();
		Init(props); // initializes the window
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown(); // closes the window
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		auto time = CREATE_PROFILEI();
		m_Data.Title = props.Title; // sets the window title
		m_Data.Width = props.Width; // sets the width of the window
		m_Data.Height = props.Height; // sets the hight of the window
		m_Data.Maximized = props.Maximized;
		m_Data.VSync = props.VSync;

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
		DWORD style = WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
		CORE_ASSERT(AdjustWindowRect(&wr, style, FALSE) != 0, "Failed to adjust window rect");

		// creat the window
		const wchar_t* wtitle = GetWStr(props.Title.c_str());
		m_Window = CreateWindow(
			WindowClass::GetName(), wtitle, style,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
			nullptr, nullptr, WindowClass::GetInstance(), this
		);
		delete[] wtitle;

		// check if the windows was created successfully
		CORE_ASSERT(m_Window != nullptr, "Failed to create window");

		// show the window
		ShowWindow(m_Window, props.Maximized ? SW_MAXIMIZE : SW_SHOW);

		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();
	}

	void WindowsWindow::Shutdown()
	{
		if (m_Window)
		{
			DestroyWindow(m_Window);
			m_Window = nullptr;
		}
	}

	void WindowsWindow::OnUpdate()
	{
		MSG msg;
		BOOL gResult = PeekMessage(&msg, m_Window, 0, 0, PM_REMOVE);
		if (gResult != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		m_Context->SwapBuffers(); // swap frame buffers
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync; // returns the v-sync state
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
		// get a pointer to the window
		WindowsWindow* const pWnd = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return pWnd->HandleEvent(hWnd, msg, wParam, lParam); // call handle event for that window
	}

	LRESULT WindowsWindow::HandleEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

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
			Math::Vector2 PrevMousePos = Input::GetPreviousMousePosition();
			MouseMovedEvent* deltaMousePostionEvent = new MouseMovedEvent(MOUSE_DELTA, (float)pt.x - PrevMousePos.x, (float)pt.y - PrevMousePos.y); // creates new mouse moved event
			m_Data.EventCallback(*deltaMousePostionEvent);
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
				m_Data.Maximized = false;
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

	WindowsWindow::WindowClass::WindowClass() :
		hInst(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WindowsWindow::HandleEventSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetInstance();
		wc.hIcon = nullptr;
		wc.hIconSm = nullptr;
		wc.hbrBackground = nullptr;
		wc.hCursor = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = GetName();

		RegisterClassEx(&wc);
	}

	WindowsWindow::WindowClass::~WindowClass()
	{
		UnregisterClass(wndClassName, GetInstance());
	}

}
