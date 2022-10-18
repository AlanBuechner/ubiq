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

		static void Lock(bool lock);

		static void Update();
	};
}
