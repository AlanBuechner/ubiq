#include "pch.h"
#include "WindowsInput.h"
#include "Engine/Application.h"

#include <GLFW/glfw3.h>

namespace Engine
{

	Input* Input::s_Instance = new WindowsInput();

	glm::vec2 WindowsInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float WindowsInput::GetMouseXImpl()
	{
		auto pos = GetMousePositionImpl();

		return pos.x;
	}

	float WindowsInput::GetMouseYImpl()
	{
		auto pos = GetMousePositionImpl();

		return pos.y;
	}
}