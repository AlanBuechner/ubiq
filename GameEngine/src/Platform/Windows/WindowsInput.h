#pragma once

#include "Engine/Core/core.h"
#include "Engine/Core/Input/Input.h"

#ifdef PLATFORM_WINDOWS

namespace Engine
{
	class WindowsInput : public Input
	{
	protected:
		virtual Math::Vector2 QueryMousePosition() override;
	};
}

#endif
