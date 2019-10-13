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

#ifdef ENABLE_ASSERTS
		int versionMajor;
		int versionMinor;
		glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
		glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
		CORE_ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Ubiq requires at least OpenGL Version 4.5!");
#endif
	}
	void OpenGLContext::SwapBuffers()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_WindowHandle);
	}
}