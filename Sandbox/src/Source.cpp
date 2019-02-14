#include <Engine.h>

class ExampleLayer : public Engine::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		
	}

	void OnUpdate() override
	{
		
	}

	void OnEvent(Engine::Event& event) override
	{
		if (event.GetEventType() == Engine::EventType::KeyPressed)
		{
			Engine::KeyPressedEvent& e = (Engine::KeyPressedEvent&)event;
			DEBUG_TRACE("{0}", (char)e.GetKeyCode());
		}
	}
};

class Sandbox : public Engine::Application
{
public:

	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushLayer(new Engine::ImGuiLayer());
		PushLayer(new Engine::InputControlerLayer());

		Engine::InputControler* input = new Engine::InputControler();

		input->BindActionEvent(KEYCODE_A, Engine::EventType::KeyPressed, BIND_ACTION(&Sandbox::thing));
		input->BindAxisEvent(KEYCODE_W, Engine::EventType::KeyPressed, BIND_AXIS(&Sandbox::thing2), 1.0f);
	}

	~Sandbox()
	{

	}

	void thing()
	{
		DEBUG_INFO("the A key was pressed");
	}

	void thing2(float i)
	{
		DEBUG_INFO("{0}", i);
	}

};

Engine::Application* Engine::CreateApplication()
{
	return new Sandbox();
}