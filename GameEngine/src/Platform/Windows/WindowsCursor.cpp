#include "pch.h"
#include "Engine/Core/Cursor.h"

#if defined(PLATFORM_WINDOWS)

#include "Win.h"
#include "WindowsWindow.h"

namespace Engine
{

	void Cursor::ConstrainToWindow(Ref<Window> window)
	{
		HWND hWnd = (HWND)window->GetNativeWindow();

		RECT winRect;
		GetWindowRect(hWnd, &winRect);

		ClipCursor(&winRect);
	}

	void Cursor::Visibility(bool v)
	{
		ShowCursor(v);
	}


	void Cursor::Lock(bool lock)
	{

	}

	void Cursor::Update()
	{
	}


}

#endif
