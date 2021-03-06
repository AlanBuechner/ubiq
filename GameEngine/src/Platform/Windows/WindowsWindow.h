#pragma once

#include "Engine/Core/Window.h"
#include "Engine/Renderer/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>

namespace Engine 
{

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override; // updates the window

		inline virtual unsigned int GetWidth() const override { return m_Data.Width; } // get the width of the window
		inline virtual unsigned int GetHeight() const override { return m_Data.Height; } // get the hight of the window

		// Window attributes
		inline virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled) override; // enables v-sync
		virtual bool IsVSync() const override; // returns wither or not v-sync is enabled

		virtual void SetViewport(float width, float height) override;
		virtual void SetAspectRatio(float aspect) override;

		inline virtual void* GetNativeWindow() const;
	private:
		virtual void Init(const WindowProps& props); // initialize the window
		virtual void Shutdown(); // closes the window
	private:
		GLFWwindow* m_Window;
		GraphicsContext* m_Context;

		struct WindowData
		{
			std::string Title; // window title
			unsigned int Width, Height; // windows width and hight
			bool VSync; // v-sync

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}