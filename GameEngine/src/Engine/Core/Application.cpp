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

#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#include "Engine/Core/Scene/Components.h"

#include <Reflection.h>
LINK_REFLECTION_DATA(GameEngine)



// asan
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#ifdef __cplusplus
extern "C"
#endif
const char* __asan_default_options() {
	// Clang reports ODR Violation errors in mbedtls/library/certs.c.
	// NEED TO REPORT THIS ISSUE
	return "detect_odr_violation=0";
}
#endif



Engine::Application* Engine::Application::s_Instance = nullptr;

namespace Engine {


	Application::Application(const std::string& name)
	{
		CREATE_PROFILE_FUNCTIONI();
		auto timer = CREATE_PROFILEI();
		CORE_ASSERT(!s_Instance, "Application Instance already exists!!!");
		s_Instance = this;

		Renderer::Init();

		START_PROFILEI(timer, "Create Window");
		Window::Init();
		Window::SetEventCallback(BIND_EVENT_FN(&Application::OnEvent)); // set the event call back
		m_Window = Window::Create({ name, 1280, 720, true, false }); // create a window
		GPUProfiler::SetTragetWindow(m_Window->GetNativeWindow());
		END_PROFILEI(timer);

		LoadProject();
	}

	Application::~Application()
	{
		m_LayerStack.Destroy();
		OnExit();
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
		std::lock_guard g(m_InputBufferMutex);
		m_InputBuffer.Push(e);
	}

	void Application::LoadProject(const fs::path& folder)
	{
		// clear asset manager before switching
		m_AssetManager.Destroy();
		m_AssetManager.Init();

		CORE_INFO("Loading project file");
		m_CurrentProject = ProjectManager::Project(folder / "Project.ubiqproj");

		CORE_INFO("Configuring Project");
		m_AssetManager.SetAssetDirectory(m_CurrentProject.GetAssetsDirectory());
		Renderer::SetDefultMaterial(m_AssetManager.GetAsset<Material>(m_CurrentProject.GetDefultMaterialID()));
	}

	void Application::Run()
	{
		CREATE_PROFILE_FUNCTIONI();
		Profiler::InstrumentationTimer timer = CREATE_PROFILEI();
		CORE_INFO("Running Application");
		
		while (m_Running)
		{
			CREATE_PROFILE_SCOPEI("Frame");
			FrameMark;

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
				START_PROFILEI(timer, "ImGui Render");
				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_ImGuiLayer->End();
				END_PROFILEI(timer);
			}

			// update the layer stack
			START_PROFILEI(timer, "Update Layers");
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
			END_PROFILEI(timer);

			// begin rendering
			DebugRenderer::EndScene();
			Renderer::BeginFrame();

			if (!m_Minimized)
			{
				START_PROFILEI(timer, "Render Layers");
				for (Layer* layer : m_LayerStack)
					layer->OnRender();
				END_PROFILEI(timer);
			}

			// end Rendering
			Renderer::EndFrame();
			WindowManager::UpdateSwapChainBackBufferIndex();

			// clean assets
			m_AssetManager.Clean();
		}
		Renderer::WaitForRender();
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
		// copy input event buffer
		m_InputBufferMutex.lock();
		Utils::Vector<Event*> events = m_InputBuffer;
		m_InputBuffer.Clear();
		m_InputBufferMutex.unlock();

		CREATE_PROFILE_FUNCTIONI();
		for (Event* i : events)
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

		for (uint32 k = 0; k < Engine::KeyCode::SIZE; k += 1)
		{
			if (Input::GetKeyPressed(k) || Input::GetKeyDown(k))
				events.Push(new KeyDownEvent(k));
		}


		for (Event** i = &*events.begin(); i != &*events.end(); ++i)
		{
			for (Layer** it = m_LayerStack.end(); it != m_LayerStack.begin(); )
			{
				(*--it)->OnEvent(*i);
				if ((*i)->Handled)
					break;
			}
			delete *i;
		}

		
	}

	bool Application::OnWindowClose(WindowCloseEvent* e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent* e)
	{
		Renderer::WaitForRender();

		m_Window->GetSwapChain()->Resize(e->GetWidth(), e->GetHeight());

		if (e->GetWidth() == 0 || e->GetHeight() == 0)
			m_Minimized = true;
		else
			m_Minimized = false;

		return false;
	}

}
