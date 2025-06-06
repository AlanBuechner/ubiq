#include "pch.h"
#include "Window.h"
#include "Engine/Core/Threading/Flag.h"

#include "Platform/Windows/WindowsWindow.h"

Engine::Window::EventCallbackFn Engine::Window::s_EventCallback;
Utils::Vector<Engine::Ref<Engine::Window>> Engine::WindowManager::s_Windows;

namespace Engine
{
	Ref<Window> Window::Create(const WindowProps& props)
	{
		Ref<Window> window;
#ifdef PLATFORM_WINDOWS
		window = CreateRef<WindowsWindow>();
#endif

		Flag createFlag;
		std::thread windowThread([&]() {
			window->Setup(props);
			createFlag.Signal();
			window->HandleEvents(); // start event loop
		});

		createFlag.Wait(true); // wait for window to finish being created
		windowThread.detach(); // allow thread to continue in the background

		WindowManager::AddWindow(window);
		return window;
	}

	void WindowManager::Destroy()
	{
		s_Windows.Clear();
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
