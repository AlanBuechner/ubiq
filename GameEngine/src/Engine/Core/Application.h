#pragma once

#include "core.h"

#include "Window.h"
#include "LayerStack.h"

#include "Engine/imGui/ImGuiLayer.h"

namespace Engine
{
	class Layer;
	class ImGuiLayer;
	class WindowCloseEvent;
	class WindowResizeEvent;
}

namespace Engine
{
	class ENGINE_API Application
	{
	public:
		Application(const std::string& name = "");
		virtual ~Application();

		void Run();

		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline static bool InEditer() { return s_Instance->m_InEditer; }
		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window;  }

	protected:
		bool m_InEditer = false;
		void GenLayerStack();

	private:
		void SendInputBuffer();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		Scope<Window> m_Window;
		std::vector<Event*> m_InputBuffer;
		bool m_Running = true;
		bool m_Minimized = false;

		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;

	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();
}
