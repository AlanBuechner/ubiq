#include <Engine.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
glm::mat4 camera(float Translate, glm::vec2 const & Rotate)
{
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.f);
	glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
	View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	return Projection * View * Model;
}

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