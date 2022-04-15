#pragma once
#include "Engine/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Engine
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(void* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		void* m_WindowHandle;
	};
}
