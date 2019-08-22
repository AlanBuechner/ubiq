#pragma once

#include "Engine/core.h"
#include "Engine/Input.h"

#ifdef PLATFORM_WINDOWS

namespace Engine
{
	class WindowsInput : public Input
	{
	protected:
		virtual glm::vec2 QueryMousePosition() override;
	};
}

#endif