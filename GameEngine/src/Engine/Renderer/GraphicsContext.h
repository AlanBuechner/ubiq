#pragma once
#include "Engine/Core/core.h"
#include "Engine/Core/Log.h"

namespace Engine
{
	class GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}