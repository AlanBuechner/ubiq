#pragma once

#include "Engine/core.h"
#include "Engine/Input.h"

namespace Engine
{
	class WindowsInput : public Input
	{
	protected:
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}