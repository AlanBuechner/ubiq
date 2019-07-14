#include "pch.h"
#include "Time.h"
#include "Platform/OpenGL/OpenGLTime.h"

namespace Engine
{
	Time* Time::s_Instance = Time::Create();

	Time* Time::Create()
	{
		return new OpenGLTime();
	}
}