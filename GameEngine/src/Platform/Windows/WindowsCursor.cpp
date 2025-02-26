#include "pch.h"
#include "Engine/Core/Cursor.h"
#include "Engine/Core/Input/Input.h"
#include "Engine/Core/Application.h"

#if defined(PLATFORM_WINDOWS)

#include "Win.h"
#include "WindowsWindow.h"

namespace Engine
{

	static POINT s_CursorLockPoint;
	static bool s_IsLocked;

	void Cursor::ConstrainToWindow(Ref<Window> window)
	{
		HWND hWnd = (HWND)window->GetNativeWindow();

		RECT winRect;
		GetWindowRect(hWnd, &winRect);

		ClipCursor(&winRect);
	}

	void Cursor::Visibility(bool v)
	{
		static bool visible = true;
		if (v != visible)
		{
			ShowCursor(v);
			visible = v;
		}
	}


	void Cursor::SetLockPos(int x, int y)
	{
		s_CursorLockPoint.x = x;
		s_CursorLockPoint.y = y;
	}

	void Cursor::Lock(bool lock)
	{
		if (lock != s_IsLocked)
		{
			s_IsLocked = lock;
			if(lock) // on lock
				Input::SetMousePosition({ s_CursorLockPoint.x, s_CursorLockPoint.y });
		}
	}

	void Cursor::Update()
	{
		if (s_IsLocked)
		{
			HWND window = (HWND)Engine::Application::Get().GetWindow().GetNativeWindow();
			POINT point = s_CursorLockPoint;
			ClientToScreen(window, &point);
			SetCursorPos(point.x, point.y);
		}
	}


}

#endif
