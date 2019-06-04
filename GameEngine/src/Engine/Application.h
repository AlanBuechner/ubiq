#pragma once

#include "core.h"

#include "Window.h"
#include "LayerStack.h"

// temp
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Buffer.h"

namespace Engine
{
	class Layer;
	class ImGuiLayer;
	class Event;
	class WindowCloseEvent;

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

		std::unique_ptr<Window> m_Window;
		std::vector<Event*> inputBuffer;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		unsigned int m_VertexArray;

		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;

	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();
}
