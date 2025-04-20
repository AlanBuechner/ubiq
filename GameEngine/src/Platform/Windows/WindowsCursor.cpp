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

	struct WrapBox
	{
		POINT min;
		POINT max;
	};
	static WrapBox s_WrapBox;
	static bool s_IsWrap;

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



	void Cursor::SetWrapBox(int minx, int miny, int maxx, int maxy, bool screenToClient)
	{
		s_WrapBox.min.x = minx;
		s_WrapBox.min.y = miny;
		s_WrapBox.max.x = maxx;
		s_WrapBox.max.y = maxy;

		if (screenToClient)
		{
			HWND window = (HWND)Engine::Application::Get().GetWindow().GetNativeWindow();
			ScreenToClient(window, &s_WrapBox.min);
			ScreenToClient(window, &s_WrapBox.max);
		}
	}

	void Cursor::Wrap(bool wrap)
	{
		s_IsWrap = wrap;
	}

	Math::Vector2 Cursor::WrapMouse(Math::Vector2 mousePos)
	{
		if (!s_IsWrap)
			return mousePos;

		// x
		if (mousePos.x <= s_WrapBox.min.x)
			mousePos.x = s_WrapBox.max.x + (mousePos.x - s_WrapBox.min.x) - 1;
		if (mousePos.x >= s_WrapBox.max.x)
			mousePos.x = s_WrapBox.min.x + (mousePos.x - s_WrapBox.max.x) + 1;

		// y
		if (mousePos.y <= s_WrapBox.min.y)
			mousePos.y = s_WrapBox.max.y + (mousePos.y - s_WrapBox.min.y) - 1;
		if (mousePos.y >= s_WrapBox.max.y)
			mousePos.y = s_WrapBox.min.y + (mousePos.y - s_WrapBox.max.y) + 1;

		return mousePos;
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
