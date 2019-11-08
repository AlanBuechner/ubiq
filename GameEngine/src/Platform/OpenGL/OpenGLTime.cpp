#include "pch.h"
#include "OpenGLTime.h"
#include "GLFW/glfw3.h"

namespace Engine
{
	void OpenGLTime::UpdateDeltaTimeImpl()
	{
		float time = (float)glfwGetTime();
		m_DeltaTime = time - m_LastFrameTime;
		m_LastFrameTime = time;
	}
	double OpenGLTime::GetTimeImpl()
	{
		return (double)glfwGetTime();
	}
}