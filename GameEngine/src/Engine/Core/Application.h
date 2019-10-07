#pragma once

#include "core.h"

#include "Window.h"
#include "LayerStack.h"

namespace Engine
{
	class Layer;
	class ImGuiLayer;
	class Event;
	class WindowCloseEvent;
	class WindowResizeEvent;
}

namespace Engine
{
	class ENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window;  }

	private:
		void GenLayerStack();
		void SendInputBuffer();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		Scope<Window> m_Window;
		std::vector<Event*> m_InputBuffer;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;

	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();
}
