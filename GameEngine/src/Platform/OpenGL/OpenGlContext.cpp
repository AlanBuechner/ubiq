#include "pch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Engine
{
	OpenGLContext::OpenGLContext(GLFWwindow * windowHandle)
		: m_WindowHandle(windowHandle)
	{
		CORE_ASSERT(m_WindowHandle, "handle is null")
	}
	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle); // makes the new window the current context
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // loads glad
		CORE_ASSERT(status, "Failed to initialize Glad!"); // checks if glad was seccessfuly loaded
	}
	void OpenGLContext::SwapBuffers()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_WindowHandle);
	}
}