#include "pch.h"
#include "Application.h"
#include "Core/Time.h"
#include "Input.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/imGui/ImGuiLayer.h"

#include "Renderer/Renderer.h"

namespace Engine {

#define BIND_EVENT_FN_EXTERN(x, p) std::bind(x, p, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		CORE_ASSERT(!s_Instance, "Application Instance already exists!!!")
			s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create()); // create a window
		m_Window->SetEventCallback(BIND_EVENT_FN(&Application::OnEvent)); // set the event call back

		Renderer::Init();

		GenLayerStack(); // generate the starting layer stack
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
		m_InputBuffer.insert(m_InputBuffer.begin(), &e);
	}

	void Application::Run()
	{
		CORE_INFO("Runing Application");
		while (m_Running)
		{
			Time::UpdateDeltaTime();

			Input::UpdateKeyState(); // update the key stats
			SendInputBuffer(); // sent the input buffer through the layer stack

			// update the layer stack
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			// render im gui layer
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			// update the window
			m_Window->OnUpdate();
		}
	}

	void Application::GenLayerStack()
	{
		// create im gui layer
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	void Application::SendInputBuffer()
	{
		if (m_InputBuffer.empty()) return;

		for (auto i = m_InputBuffer.end(); i != m_InputBuffer.begin();)
		{
			Event& e = *(*--i);
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(&Application::OnWindowClose));
			dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyPressed, Input::s_Instance));
			dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyReleased, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMousePressed, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMouseReleased, Input::s_Instance));
		}

		Input::GetUpdatedEventList(m_InputBuffer);

		for (auto i = m_InputBuffer.end(); i != m_InputBuffer.begin();)
		{
			Event& e = *(*--i);
			for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
			{
				(*--it)->OnEvent(e);
				if (e.Handled)
					break;
			}
			delete *i;
		}
		m_InputBuffer.clear();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}