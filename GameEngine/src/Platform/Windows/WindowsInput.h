#pragma once

#include "Engine/core.h"
#include "Engine/Input.h"

#ifdef PLATFORM_WINDOWS

namespace Engine
{
	class WindowsInput : public Input
	{
	protected:
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}

#endif