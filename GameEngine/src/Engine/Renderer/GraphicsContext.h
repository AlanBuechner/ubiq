#pragma once
#include "Engine/core.h"
#include "Engine/Log.h"

namespace Engine
{
	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}