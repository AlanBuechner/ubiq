#pragma once
#include "Engine/Core/Time.h"

namespace Engine
{
	class OpenGLTime : public Time
	{
		virtual void UpdateDeltaTimeImpl() override;
		virtual double GetTimeImpl() override;
	};
}