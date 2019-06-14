#include "pch.h"
#include "Application.h"

#include "Input.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/imGui/ImGuiLayer.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Engine {

#define BIND_EVENT_FN_EXTERN(x, p) std::bind(x, p, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		CORE_ASSERT(!s_Instance, "Application Instance already exists!!!")
			s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create()); // create a window
		m_Window->SetEventCallback(BIND_EVENT_FN(&Application::OnEvent)); // set the event call back

		GenLayerStack(); // generate the starting layer stack

		m_VertexArray.reset(VertexArray::Create());

		float vertices[4 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
		};

		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"}
		};

		m_VertexBuffer->SetLayout(layout);

		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		uint32_t indeces[] = { 0, 1, 2,  0, 3, 2 };

		m_IndexBuffer.reset(IndexBuffer::Create(indeces, sizeof(indeces) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);

		Shader::ShaderSorce src;
		src << Shader::LoadShader("C:\\Users\\Alan\\source\\repos\\GameEngine\\shader.hlsl");

		m_Shader.reset(Shader::Create(src.vertexShader, src.pixleShader));
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
		CORE_INFO("Runing Application");
		while (m_Running)
		{
			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->Bind();
			m_VertexArray->Bind();
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

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
		if (inputBuffer.empty()) return; // 

		for (auto i = inputBuffer.end(); i != inputBuffer.begin();)
		{
			Event& e = *(*--i);
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(&Application::OnWindowClose));
			dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyPressed, Input::s_Instance));
			dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnKeyReleased, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMousePressed, Input::s_Instance));
			dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN_EXTERN(&Input::OnMouseReleased, Input::s_Instance));
		}

		Input::GetUpdatedEventList(inputBuffer);

		for (auto i = inputBuffer.end(); i != inputBuffer.begin();)
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
		inputBuffer.clear();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}