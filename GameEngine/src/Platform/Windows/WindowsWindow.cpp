#include "pch.h"
#include "WindowsWindow.h"
#include "Engine/Core/Log.h"

#include "Engine/Core/Input/InputControler.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Core/Input/Input.h"

namespace Engine 
{

	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description)
	{
		CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props); // initializes the window
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown(); // closes the window
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title; // sets the window title
		m_Data.Width = props.Width; // sets the width of the window
		m_Data.Height = props.Height; // sets the hight of the window

		CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		// handels if the window hasent be initalized
		if (!s_GLFWInitialized)
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit(); // initalize the window
			CORE_ASSERT(success, "Could not intialize GLFW!"); // checks if the window was seccessfuly initalized
			glfwSetErrorCallback(GLFWErrorCallback); // sets the error callback
			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr); // creates the window

		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true); // sets v-sync to true

		// set GLFW callbacks
		// set resize event
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window); // gets window data
			data.Width = width; // sets new width
			data.Height = height; // sets new hight

			WindowResizeEvent* event = new WindowResizeEvent(width, height); // creates new resize event
			data.EventCallback(*event); //
		});

		// set close event
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window); // gets window data
			WindowCloseEvent* event = new WindowCloseEvent(); // creats new close event
			data.EventCallback(*event);
		});

		// set key events
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window); // gets window data

			key = KeyCode::OglToUbiq(key);

			switch (action) // determens the type of key event
			{
			case GLFW_PRESS: // press event
			{
				KeyPressedEvent* event = new KeyPressedEvent(key); // creats new key pressed event
				data.EventCallback(*event);
				break;
			}
			case GLFW_RELEASE: // release event
			{
				KeyReleasedEvent* event = new KeyReleasedEvent(key); // creates new key release event
				data.EventCallback(*event);
				break;
			}
			case GLFW_REPEAT: // repeat event
			{
				KeyRepeatEvent* event = new KeyRepeatEvent(key, 1); // creates new key repeat event
				data.EventCallback(*event);
				break;
			}
			}
		});

		// set char event
		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window); // gets window data
			keycode = KeyCode::OglToUbiq(keycode);
			KeyTypedEvent* event = new KeyTypedEvent(keycode); // creates new key typed event
			data.EventCallback(*event);
		});

		// set mouse button event
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window); // gets window data

			button = KeyCode::OglToUbiqMouse(button);

			switch (action) // determens the type of mouse button event
			{
			case GLFW_PRESS: // press event
			{
				MouseButtonPressedEvent* event = new MouseButtonPressedEvent(button); // creates new mouse button pressed event
				data.EventCallback(*event);
				break;
			}
			case GLFW_RELEASE: // release event
			{
				MouseButtonReleasedEvent* event = new MouseButtonReleasedEvent(button); // creates new mouse button release event
				data.EventCallback(*event);
				break;
			}
			}
		});

		// set scroll event
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window); // gets window data

			MouseScrolledEvent* event = new MouseScrolledEvent(MOUSE_SCROLL_WHEEL, (float)xOffset, (float)yOffset); // creates new mouse scrolled event
			data.EventCallback(*event);
		});

		// set mouse moved event
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window); // gets window data

			MouseMovedEvent* cursorPositionEvent = new MouseMovedEvent(MOUSE_POSITON, (float)xPos, (float)yPos); // creates new mouse moved event
			data.EventCallback(*cursorPositionEvent);

			glm::vec2 PrevMousePos = Input::GetPreviousMousePosition();
			MouseMovedEvent* deltaMousePostionEvent = new MouseMovedEvent(MOUSE_DELTA, (float)xPos - PrevMousePos.x , (float)yPos - PrevMousePos.y); // creates new mouse moved event
			data.EventCallback(*deltaMousePostionEvent);
		});
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window); // destroys the window
	}

	void WindowsWindow::OnUpdate()
	{
		m_Context->SwapBuffers(); // swap frame buffers
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync; // returns the v-sync state
	}
	inline void* WindowsWindow::GetNativeWindow() const
	{
		return m_Window;
	}
}