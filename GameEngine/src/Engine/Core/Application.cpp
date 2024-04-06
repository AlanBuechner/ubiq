#include "pch.h"
#include "Application.h"
#include "Time.h"
#include "Input/Input.h"
#include "Cursor.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"

#include "Engine/imGui/ImGuiLayer.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Abstractions/Resources/SwapChain.h"
#include "Engine/Renderer/Abstractions/Resources/ResourceManager.h"

#include "Engine/Util/Performance.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#include <Reflection.h>
LINK_REFLECTION_DATA(GameEngine)

Engine::Application* Engine::Application::s_Instance = nullptr;

namespace Engine {


	Application::Application(const std::string& name)
	{
		CREATE_PROFILE_FUNCTIONI();
		auto timer = CREATE_PROFILEI();
		CORE_ASSERT(!s_Instance, "Application Instance already exists!!!");
		s_Instance = this;

		Renderer::Init();

		timer.Start("Create Window");
		Window::Init();
		m_Window = Window::Create({ name, 1280, 720, true, false }); // create a window
		GPUProfiler::SetTragetWindow(m_Window->GetNativeWindow());
		timer.End();
		timer.Start("set event callback");
		m_Window->SetEventCallback(BIND_EVENT_FN(&Application::OnEvent)); // set the event call back
		timer.End();

		m_AssetManager.Init();
	}

	Application::~Application()
	{
		m_LayerStack.RemoveAllLayers();
		m_AssetManager.Destroy();
		m_Window.reset();
		WindowManager::Destroy();
		Window::Shutdown();
		
		Renderer::Destroy();
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
		//const Reflect::Class& c = Reflect::Registry::GetRegistry()->GetClass("Engine::TransformComponent");

		//CORE_INFO("{0}", c.GetName());

		CREATE_PROFILE_FUNCTIONI();
		InstrumentationTimer timer = CREATE_PROFILEI();
		CORE_INFO("Runing Application");
		
		while (m_Running)
		{
			CREATE_PROFILE_SCOPEI("Frame");
			Time::UpdateDeltaTime();

			//Engine::Instrumentor::Get().RecordData(false);

			//CORE_INFO("{0}", Time::GetFPS());

			Window::HandleEvents();
			Input::UpdateKeyState(); // update the key stats
			SendInputBuffer(); // sent the input buffer through the layer stack
			Cursor::Update();

			// update gui
			if (m_InEditer)
			{
				// render imgui layer
				timer.Start("ImGui Render");
				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				Performance::Render();
				m_ImGuiLayer->End();
				timer.End();
			}

			// update the layer stack
			timer.Start("Update Layers");
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
			timer.End();

			// begin rendering
			Renderer::BeginFrame();

			if (!m_Minimized)
			{
				timer.Start("Render Layers");
				for (Layer* layer : m_LayerStack)
					layer->OnRender();
				timer.End();
			}

			if (m_InEditer)
				m_ImGuiLayer->Build();

			// end Rendering
			Renderer::EndFrame();
			WindowManager::UpdateSwapChainBackBufferIndex();

			// clean assets
			m_AssetManager.Clean();

			//Engine::Instrumentor::Get().RecordData(true);
		}
		Renderer::WaitForSwap(); // wait for last frame to finish
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::GenLayerStack()
	{
		if (m_InEditer)
		{
			// create imgui layer
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
			dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMouseMoved, Input::s_Instance));
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

		return false;
	}

}
