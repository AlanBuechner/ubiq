#include "pch.h"
#include "Window.h"

#include "Platform/Windows/WindowsWindow.h"

Utils::Vector<Engine::Ref<Engine::Window>> Engine::WindowManager::s_Windows;

namespace Engine
{
	Ref<Window> Window::Create(const WindowProps& props)
	{
		Ref<Window> window;
#ifdef PLATFORM_WINDOWS
		window = CreateRef<WindowsWindow>(props);
#endif

		WindowManager::AddWindow(window);
		return window;
	}

	void WindowManager::Destroy()
	{
		s_Windows.Destroy();
	}

	void WindowManager::UpdateWindows()
	{
		for (auto window : s_Windows)
			window->OnUpdate();
	}

	void WindowManager::UpdateSwapChainBackBufferIndex()
	{
		for (auto window : s_Windows)
			window->GetSwapChain()->UpdateBackBufferIndex();
	}

	void WindowManager::AddWindow(Ref<Window> window)
	{
		s_Windows.Push(window);
	}


}
