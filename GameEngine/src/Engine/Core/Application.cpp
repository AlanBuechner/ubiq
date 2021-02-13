#include "pch.h"
#include "Application.h"
#include "Time.h"
#include "Input/Input.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"

#include "Engine/imGui/ImGuiLayer.h"

#include "Engine/Renderer/Renderer.h"

#include "Engine/Util/Performance.h"

namespace Engine {

#define BIND_EVENT_FN_EXTERN(x, p) std::bind(x, p, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{

		CREATE_PROFILE_FUNCTIONI();
		auto timer = CREATE_PROFILEI();
		CORE_ASSERT(!s_Instance, "Application Instance already exists!!!");
		s_Instance = this;

		timer.Start("Create Window");
		m_Window = std::unique_ptr<Window>(Window::Create(name)); // create a window
		timer.End();
		timer.Start("set event callback");
		m_Window->SetEventCallback(BIND_EVENT_FN(&Application::OnEvent)); // set the event call back
		timer.End();

		timer.Start("Renderer Init");
		Renderer::Init();
		timer.End();
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
		CREATE_PROFILE_FUNCTIONI();
		Engine::InstrumentationTimer timer = CREATE_PROFILEI();
		CORE_INFO("Runing Application");
		

		while (m_Running)
		{
			CREATE_PROFILE_SCOPEI("Frame");
			Time::UpdateDeltaTime();

			timer.Start("Handle Input");
			Input::UpdateKeyState(); // update the key stats
			SendInputBuffer(); // sent the input buffer through the layer stack
			timer.End();

			timer.Start("Update Layers");
			// update the layer stack
			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate();
			}
			timer.End();

			timer.Start("ImGui Render");
			// render im gui layer
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			Performance::Render();
			m_ImGuiLayer->End();
			timer.End();

			timer.Start("Update the Window");
			// update the window
			m_Window->OnUpdate();
			timer.End();
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::GenLayerStack()
	{
		if (m_InEditer)
		{
			// create im gui layer
			m_ImGuiLayer = new ImGuiLayer();
			PushOverlay(m_ImGuiLayer);
		}
	}

	void Application::SendInputBuffer()
	{
		if (m_InputBuffer.empty()) return;

		for (auto i = m_InputBuffer.end(); i != m_InputBuffer.begin();)
		{
			Event& e = *(*--i);
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(&Application::OnWindowClose));
			dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(&Application::OnWindowResize));
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
			delete* i;
		}

		m_InputBuffer.clear();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
			m_Minimized = true;
		else
			m_Minimized = false;

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}