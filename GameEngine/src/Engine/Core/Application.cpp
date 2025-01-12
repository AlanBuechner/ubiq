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
#include "Engine/Renderer/DebugRenderer.h"

#include "Engine/Util/Performance.h"

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#include "Engine/Core/Scene/Components.h"

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

		CORE_INFO("Loading project file");
		m_CurrentProject = ProjectManager::Project("Project.ubiqproj");
			
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

		CORE_INFO("Configuring Project");
		m_AssetManager.AddAssetDirectory(m_CurrentProject.GetAssetsDirectory());
		Renderer::SetDefultMaterial(m_AssetManager.GetAsset<Material>(m_CurrentProject.GetDefultMaterialID()));
	}

	Application::~Application()
	{
		m_LayerStack.Destroy();
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

	void Application::OnEvent(Event* e)
	{
		m_InputBuffer.Push(e);
	}

	void Application::Run()
	{
		CREATE_PROFILE_FUNCTIONI();
		InstrumentationTimer timer = CREATE_PROFILEI();
		CORE_INFO("Runing Application");
		
		while (m_Running)
		{
			CREATE_PROFILE_SCOPEI("Frame");

			m_LayerStack.UpdateLayerChanges();

			Time::UpdateDeltaTime();

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
			DebugRenderer::EndScene();
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
		if (m_InputBuffer.Empty()) return;

		for (Event* i : m_InputBuffer)
		{
			EventDispatcher dispatcher(i);
			dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(&Application::OnWindowClose));
			dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(&Application::OnWindowResize));
			dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyPressed, Input::s_Instance));
			dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyReleased, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMousePressed, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMouseReleased, Input::s_Instance));
			dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMouseMoved, Input::s_Instance));
		}

		for (Event** i = &*m_InputBuffer.begin(); i != &*m_InputBuffer.end(); ++i)
		{
			for (Layer** it = m_LayerStack.end(); it != m_LayerStack.begin(); )
			{
				(*--it)->OnEvent(*i);
				if ((*i)->Handled)
					break;
			}
			delete *i;
		}

		m_InputBuffer.Clear();
	}

	bool Application::OnWindowClose(WindowCloseEvent* e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent* e)
	{
		if (e->GetWidth() == 0 || e->GetHeight() == 0)
			m_Minimized = true;
		else
			m_Minimized = false;

		return false;
	}

}
