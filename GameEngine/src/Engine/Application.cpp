#include "pch.h"
#include "Application.h"

#include "Engine/Log.h"

#include "Input.h"

namespace Engine {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
#define BIND_EVENT_FN_EXTERN(x, p) std::bind(x, p, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		CORE_ASSERT(!s_Instance, "Application Instance already exists!!!")
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		
		GenLayerStack();
	}

	Application::~Application()
	{
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	void Application::OnEvent(Event& e)
	{
		inputBuffer.insert(inputBuffer.begin(), &e);
	}

	void Application::Run()
	{
		while (m_Running)
		{
			Input::UpdateKeyState();

			SendInputBuffer();

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::GenLayerStack()
	{
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	void Application::SendInputBuffer()
	{
		if (inputBuffer.empty()) return;

		for (auto i = inputBuffer.end(); i != inputBuffer.begin();)
		{
			Event& e = *(*--i);
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
			dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyPressed, Input::s_Instance));
			dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyReleased, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMousePressed, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMouseReleased, Input::s_Instance));

			for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
			{
				(*--it)->OnEvent(e);
				if (e.Handled)
					break;
			}
			delete *i;
		}
		inputBuffer.clear();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}