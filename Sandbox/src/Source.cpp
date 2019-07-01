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
	Engine::EventData* boundedKey;
	InputControler* input;

	bool set = false;
	ExampleLayer()
		: Layer("Example")
	{
		input = new InputControler(m_InputManeger);

		input->BindEvent(MOUSE_LBUTTON, MOUSE_PRESSED, BIND_ACTION(&ExampleLayer::thing));
		boundedKey = input->BindEvent(KEYCODE_W, KEY_DOWN, BIND_AXIS(&ExampleLayer::thing2, true));
		input->BindEvent(KEYCODE_S, KEY_DOWN, BIND_AXIS(&ExampleLayer::thing2, false));

	}

	void OnUpdate() override
	{
		//DEBUG_INFO("{0}", Input::GetKeyState(KEYCODE_F));
		//DEBUG_INFO("{0}", Input::GetMouseButtonState(MOUSE_LBUTTON));
		Input::GetKeyDown(KEYCODE_W);
	}

	void OnEvent(Engine::Event& event) override
	{
		Engine::Layer::OnEvent(event);
		if (event.GetEventType() == Engine::EventType::KeyPressed)
		{
			Engine::KeyPressedEvent& e = (Engine::KeyPressedEvent&)event;
		}
	}

	void thing()
	{
		DEBUG_INFO("the mouse button was pressed");
	}

	void thing2(bool i)
	{

		DEBUG_INFO("{0}", i);
		if (i == false)
		{
			delete input;
		}
	}
};

class Sandbox : public Engine::Application
{
public:

	Sandbox()
	{
		PushLayer(new ExampleLayer());

		
	}

	~Sandbox()
	{

	}

};

Engine::Application* Engine::CreateApplication()
{
	return new Sandbox();
}