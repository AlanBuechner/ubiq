#pragma once

#include "Core.h"
#include "Engine/AssetManager/AssetManager.h"

#include "Window.h"
#include "LayerStack.h"

#include "Engine/imGui/ImGuiLayer.h"

#include "ProjectManager/Project.h"

namespace Engine
{
	class Layer;
	class ImGuiLayer;
	class WindowCloseEvent;
	class WindowResizeEvent;
	class Application;
}

namespace Engine
{
	class  Application
	{
	public:
		Application(const std::string& name = "");
		virtual ~Application();
		inline static Application& Get() { return *s_Instance; }
		template<class T>
		inline static T& GetAs() { return *(T*)s_Instance; }

		void Run();

		void Close();

		void OnEvent(Event* e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static bool InEditer() { return s_Instance->m_InEditer; }

		inline Window& GetWindow() { return *m_Window;  }
		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		inline LayerStack& GetLayerStack() { return m_LayerStack; }
		inline AssetManager& GetAssetManager() { return m_AssetManager; }

		inline ProjectManager::Project& GetProject() { return m_CurrentProject; }

		inline static bool IsRunning() { return s_Instance->m_Running; }

	protected:
		bool m_InEditer = false;
		void GenLayerStack();

	private:
		void SendInputBuffer();

		bool OnWindowClose(WindowCloseEvent* e);
		bool OnWindowResize(WindowResizeEvent* e);

		Ref<Window> m_Window;
		Utils::Vector<Event*> m_InputBuffer;
		bool m_Running = true;
		bool m_Minimized = false;

		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
		Utils::Vector<Layer*> m_NewLayers;


		AssetManager m_AssetManager;

		ProjectManager::Project m_CurrentProject;

	private:
		static Application* s_Instance;
	};
}
