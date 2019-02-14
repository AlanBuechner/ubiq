#include "pch.h"
#include "WindowsInput.h"
#include "Engine/Application.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	KeyCode* KeyCode::s_Instance = new WindowsKeyCode();

	int WindowsKeyCode::GetKeyCodeImpl(char key)
	{
		int code = (int)key;
		if ((code >= 56 && code <= 90) || (code >= 48 && code <= 57))
		{
			return code;
		}
		else if (code >= 97 && code <= 122)
		{
			return code - 41;
		}
	}
}

namespace Engine
{

	Input* Input::s_Instance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);

		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float WindowsInput::GetMouseXImpl()
	{
		auto[x, y] = GetMousePositionImpl();

		return x;
	}

	float WindowsInput::GetMouseYImpl()
	{
		auto[x, y] = GetMousePositionImpl();

		return y;
	}
}