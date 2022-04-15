#include "pch.h"
#include "OpenGLContext.h"
#include "Engine/Core/Application.h"

#include <Windows.h>
#include <glad/glad.h>
#include <glad/glad_wgl.h>

namespace Engine
{
	OpenGLContext::OpenGLContext(void* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		CORE_ASSERT(m_WindowHandle, "handle is null")
	}
	void OpenGLContext::Init()
	{
#if defined(PLATFORM_WINDOWS)

		HDC dc = GetDC((HWND)m_WindowHandle);

		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.cColorBits = 32;
		pfd.cAlphaBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;

		int pixelFormat = ChoosePixelFormat(dc, &pfd);
		SetPixelFormat(dc, pixelFormat, &pfd);

		HGLRC rc = wglCreateContext(dc);
		wglMakeCurrent(dc, rc);

		CORE_ASSERT(gladLoadWGL(wglGetCurrentDC()), "gladLoadWGL failed");
		CORE_ASSERT(gladLoadGL(), "gladLoadGL failed");
#endif

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
		wglSwapIntervalEXT(Application::Get().GetWindow().IsVSync());
		wglSwapLayerBuffers(GetDC(HWND(Application::Get().GetWindow().GetNativeWindow())), WGL_SWAP_MAIN_PLANE);
	}

}
