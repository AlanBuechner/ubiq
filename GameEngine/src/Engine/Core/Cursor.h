#pragma once
#include "Core.h"
#include "Window.h"

namespace Engine
{
	class Cursor
	{
	public:
		static void ConstrainToWindow(Ref<Window> window);
		static void Visibility(bool v);

		static void SetWrapBox(int minx, int miny, int maxx, int maxy, bool screenToClient = false);
		static void Wrap(bool wrap);
		static Math::Vector2 WrapMouse(Math::Vector2 mousePos);

		static void SetLockPos(int x, int y);
		static void Lock(bool lock);

		static void Update();
	};
}
